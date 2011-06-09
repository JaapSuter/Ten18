#include "Ten18/PCH.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Interop/HostControl.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Tracer.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
    
using namespace Ten18::Interop;
using namespace Ten18::COM;

namespace Ten18 {
    
Host::Host() :
    mControl(*this), 
    mAssemblyManager(*this),
    mAssemblyStore(*this),
    mMemoryManager(*this),
    mHostGCManager(*this),
    mMetaHost(),
    mRuntimeInfo(),
    mRuntimeHost(),
    mClrControl(),
    mGCManager(),
    mHeart()    
{
    const auto dotNetRuntimeVersion = L"v4.0.30319";   

    Expect.HR = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void**>(&mMetaHost));
        
    Expect.HR = mMetaHost->GetRuntime(dotNetRuntimeVersion, IID_ICLRRuntimeInfo, reinterpret_cast<void**>(&mRuntimeInfo));
    Expect.HR = mRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, reinterpret_cast<void**>(&mRuntimeHost));
    Expect.HR = mRuntimeHost->GetCLRControl(&mClrControl);
    Expect.HR = mRuntimeHost->SetHostControl(&mControl);
    
    const auto startupFlags = STARTUP_SINGLE_VERSION_HOSTING_INTERFACE
                            | STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN
                            | STARTUP_LOADER_SAFEMODE
                            | STARTUP_SERVER_GC
                            | STARTUP_ARM;
                            
    Expect.HR = mRuntimeInfo->SetDefaultStartupFlags(startupFlags, nullptr);
            
    Expect.HR = mClrControl->GetCLRManager(IID_ICLRGCManager, reinterpret_cast<void**>(&mGCManager));
    
    const DWORD segmentSize = 16 * 1024 * 1024;
    const DWORD maxGen0Size = 2 * 1024 * 1024;
    Expect.HR = mGCManager->SetGCStartupLimits(segmentSize, maxGen0Size);

    const auto assemblyName = L"Ten18.Net, Version=1.0.1.8, PublicKeyToken=39a56a431d4ba826, culture=neutral";
    const auto domainManager = L"Ten18.Interop.AppDomainManagerEx";
    Expect.HR = mClrControl->SetAppDomainManagerType(assemblyName, domainManager);

    Ten18_ASSERT(mHeart == nullptr);
    Expect.HR = mRuntimeHost->Start();
    Ten18_ASSERT(mHeart != nullptr);
    
    mHeart->Rendezvous();
}
    
Host::~Host()
{
    Ten18_ASSERT_MSG(mHeart == nullptr, "Host destructor called while heart still beating, or alive anyway...");
}

void Host::Run()
{
    MSG msg = {};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            mHeart->Beat();
        }
    }

    const auto ec = static_cast<int>(msg.wParam);
    PostQuitMessage(ec);

    Ten18_ASSERT(mHeart != nullptr);
    mHeart->Farewell();
    mHeart->Release();
    mHeart = nullptr;
    
    Expect.HR = mRuntimeHost->Stop();    
    Expect.HR = mMetaHost->ExitProcess(ec);
}

}
