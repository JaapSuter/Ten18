#include "Ten18/PCH.h"
#include "Ten18/Interop/HostAssemblyStore.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/Content/Index.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Content/Index.h"
#include "Ten18/Util.h"

using namespace Ten18;
using namespace Ten18::Interop;
using namespace Ten18::COM;

HostAssemblyStore::HostAssemblyStore(Host& host)
  : mHost(&host),
    mUniqueAssemblyIds(1018)
{}

HRESULT STDMETHODCALLTYPE HostAssemblyStore::ProvideAssembly(
        AssemblyBindInfo *pBindInfo,
        UINT64 *pAssemblyId,
        UINT64 *pContext,
        IStream **ppStmAssemblyImage,
        IStream **ppStmPDB)
{
    *pAssemblyId = 0;
    *pContext = 0;
    *ppStmAssemblyImage = nullptr;
    *ppStmPDB = nullptr;

    const std::wstring postPolicyIdentity(pBindInfo->lpPostPolicyIdentity);
    const auto idx = postPolicyIdentity.find_first_of(L',');
    if (idx == std::wstring::npos)
        return COR_E_FILENOTFOUND;
        
    const auto requestedAssemblyName = postPolicyIdentity.substr(0, idx);
    
    auto pe = Content::Index::TryGet(pBindInfo->lpPostPolicyIdentity);
    if (pe)
    {
        *ppStmAssemblyImage = new EmbeddedResourceStream(pe->Data, pe->Size);
        *pAssemblyId = reinterpret_cast<UINT64>(pe->Data);
    }
    else
    {
        // Todo, Jaap Suter, June 2011, this is rather hardcoded and ugly. It's a dev-only code
        // path, but still... also - it's late. Time to get some sleep... 
        if (requestedAssemblyName.find(L"asyncctplibrary") == std::wstring::npos)
        {
            if (postPolicyIdentity.find(L"x86") != postPolicyIdentity.size() - 3)
                return COR_E_FILENOTFOUND;
        }
        else if (postPolicyIdentity.find(L"msil") != postPolicyIdentity.size() - 4)
            return COR_E_FILENOTFOUND;
                
        auto dll = Content::Index::Root() + requestedAssemblyName + L".dll";
        if (Util::FileExists(dll.c_str()))
        {            
            Expect.HR = SHCreateStreamOnFileEx(dll.c_str(), STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, nullptr, ppStmAssemblyImage);
            static UINT64 sAssemblyIdHack = 0xDEADBEEF10181018;
            *pAssemblyId = sAssemblyIdHack++; // Todo, Jaap Suter, June 2011, this misses the point of the assembly id. But alas...
        }
        else return COR_E_FILENOTFOUND;
    }

    const auto securityCookie = 0xBA5E1018;
    *pContext = securityCookie;
    
    auto pdb = Content::Index::Root() + requestedAssemblyName + L".pdb";
    if (Util::FileExists(pdb.c_str()))
        Expect.HR = SHCreateStreamOnFileEx(pdb.c_str(), STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, nullptr, ppStmPDB);

    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostAssemblyStore::ProvideModule(
    ModuleBindInfo *pBindInfo,
    DWORD *pdwModuleId,
    IStream **ppStmModuleImage,
    IStream **ppStmPDB)
{
    UNREFERENCED_PARAMETER(pBindInfo);
        
    const int dummyIdMustNotBeZero = 1;

    *pdwModuleId = dummyIdMustNotBeZero;
    *ppStmModuleImage = nullptr;
    *ppStmPDB = nullptr;

    return COR_E_FILENOTFOUND;
}
