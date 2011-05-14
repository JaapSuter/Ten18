#include "Ten18/PCH.h"
#include "Ten18/CLR/Host.h"
#include "Ten18/CLR/HostControl.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Tracer.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/Hosting/IManagedServiceProvider.Generated.h"

using namespace Ten18::CLR;
using namespace Ten18::COM;

namespace Ten18 {
    
class HostImpl
{
public:
    HostImpl();

private:

    struct EntryPoint
    {
        EntryPoint()
            : AssemblyPath(L"Ten18.Net.dll"),                  
                AssemblyName(L"Ten18.Net, Version=1.0.0.0, PublicKeyToken=39a56a431d4ba826, culture=neutral"),
                TypeName(L"Ten18.EntryPoint"),
                MethodName(L"HostedMain"),
                DomainManager(L"Ten18.Hosting.AppDomainManager"),
                DotNetRuntimeVersion(L"v4.0.30319") {}
            
        const wchar_t* const AssemblyPath;
        const wchar_t* const AssemblyName;
        const wchar_t* const TypeName;
        const wchar_t* const MethodName;
        const wchar_t* const DomainManager;
        const wchar_t* const DotNetRuntimeVersion;
        
    private:
        EntryPoint& operator = (const EntryPoint&);
    };

    HostImpl(const HostImpl&);
    HostImpl& operator = (const HostImpl&);

    const EntryPoint mEntryPoint;
    HostControl mHostControl;
        
    // Because a process that hosts the CLR can't actually stop, unload, or otherwise deinitialize
    // the CLR (or its default AppDomain) once loaded, we just let it die with the process (which appears
    // to by as designed by the CLR hosting API). Henceforth, we're not going to bother releasing any of
    // these pointers. And that's why we don't use reference counted smart pointers, and just resort to
    // plain old fashioned raw pointers.
    ICLRMetaHost*       mMetaHost;
    ICLRRuntimeInfo*    mRuntimeInfo;
    ICLRControl*        mControl;
    ICLRGCManager*      mGCManager;     
    ICLRRuntimeHost*    mRuntimeHost;
};
    
static HostImpl* sHostImpl = nullptr;

void Host::Initialize()
{
    Ten18_TRACER();
    Ten18_ASSERT(sHostImpl == nullptr);
    if (sHostImpl == nullptr)
    {
        auto hostImpl = new HostImpl();
        Expect.EqualTo(sHostImpl) = hostImpl;
    }
}

HostImpl::HostImpl() :
    mHostControl(), 
    mMetaHost(),
    mRuntimeInfo(),
    mRuntimeHost(),
    mControl(),
    mGCManager(),
    mEntryPoint()
{
    Ten18_TRACER();

    sHostImpl = this;

    Expect.HR = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, reinterpret_cast<void**>(&mMetaHost));
        
    Expect.HR = mMetaHost->GetRuntime(mEntryPoint.DotNetRuntimeVersion, IID_ICLRRuntimeInfo, reinterpret_cast<void**>(&mRuntimeInfo));
    Expect.HR = mRuntimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, reinterpret_cast<void**>(&mRuntimeHost));
    Expect.HR = mRuntimeHost->GetCLRControl(&mControl);
    Expect.HR = mRuntimeHost->SetHostControl(&mHostControl);    
    
    const auto startupFlags = STARTUP_SINGLE_VERSION_HOSTING_INTERFACE
                            | STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN
                            | STARTUP_LOADER_SAFEMODE
                            | STARTUP_SERVER_GC
                            | STARTUP_ARM;

    Expect.HR = mRuntimeInfo->SetDefaultStartupFlags(startupFlags, nullptr);
            
    Expect.HR = mControl->GetCLRManager(IID_ICLRGCManager, reinterpret_cast<void**>(&mGCManager));

    const DWORD segmentSize = 8 * 1024 * 1024;
    const DWORD maxGen0Size = 128 * 1024;
    Expect.HR = mGCManager->SetGCStartupLimits(segmentSize, maxGen0Size);

    Expect.HR = mControl->SetAppDomainManagerType(mEntryPoint.AssemblyName, mEntryPoint.DomainManager);
        
    Expect.HR = mRuntimeHost->Start();

    Ten18_ASSERT(mHostControl.GetManagedServiceProvider() != nullptr);
    
    auto ret = mHostControl.GetManagedServiceProvider()->TwoArgsOneReturnTest(3, 4);
    DebugOut("ret: %d", ret);
}

}
