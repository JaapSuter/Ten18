#include "Ten18/PCH.h"
#include "Ten18/CLR/HostAssemblyManager.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/CLR/Host.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::CLR;
using namespace Ten18::COM;

HostAssemblyManager::HostAssemblyManager(Host& host)
    : mHost(&host)
{}

HRESULT STDMETHODCALLTYPE HostAssemblyManager::GetNonHostStoreAssemblies(ICLRAssemblyReferenceList **ppReferenceList)
{   
    ppReferenceList = nullptr;        
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostAssemblyManager::GetAssemblyStore(IHostAssemblyStore **ppAssemblyStore)
{
    *ppAssemblyStore = &mHost->mAssemblyStore;
    return S_OK;
}
