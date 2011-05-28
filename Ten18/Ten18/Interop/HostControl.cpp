#include "Ten18/PCH.h"
#include "Ten18/Interop/HostControl.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::Interop;
using namespace Ten18::COM;

HostControl::HostControl(Host& host)
  : mHost(&host)
{}

HRESULT STDMETHODCALLTYPE HostControl::GetHostManager(REFIID riid, void** ppObject)
{
    if (riid == IID_IHostAssemblyManager)
        *ppObject = static_cast<IHostAssemblyManager*>(&mHost->mAssemblyManager);
    else if (riid == IID_IHostGCManager)
        *ppObject = static_cast<IHostGCManager*>(&mHost->mHostGCManager);
    else if (riid == IID_IHostMemoryManager)
        *ppObject = static_cast<IHostMemoryManager*>(&mHost->mMemoryManager);
    else if (riid == IID_IHostAssemblyStore)
        *ppObject = static_cast<IHostAssemblyStore*>(&mHost->mAssemblyStore);
    else
        if (ppObject != nullptr)
            *ppObject = nullptr;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostControl::SetAppDomainManager(DWORD dwAppDomainID, IUnknown* appDomainManagerAsUnknown)
{
    UNREFERENCED_PARAMETER(dwAppDomainID);
    Ten18_ASSERT(dwAppDomainID == 1);

    auto& ad = mHost->mAppDomainManagerEx;

    Ten18_ASSERT(ad == nullptr);
    Expect.HR = appDomainManagerAsUnknown->QueryInterface(__uuidof(ad), reinterpret_cast<void**>(&ad));
    Ten18_ASSERT(ad != nullptr);
        
    return S_OK;
}
