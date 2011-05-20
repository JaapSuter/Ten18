#include "Ten18/PCH.h"
#include "Ten18/CLR/HostAssemblyStore.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Content/Index.h"

using namespace Ten18;
using namespace Ten18::CLR;
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

    DebugOut("pBindInfo->lpPostPolicyIdentity: %S", pBindInfo->lpPostPolicyIdentity);
    DebugOut("pBindInfo->lpReferencedIdentity: %S", pBindInfo->lpReferencedIdentity);

    auto entry = Content::Index::Get(pBindInfo->lpPostPolicyIdentity);

    const auto securityCookie = 0xBA5E1018;
    *pContext = securityCookie;
    *ppStmAssemblyImage = new EmbeddedResourceStream(entry.Data, entry.Size);
    *pAssemblyId = reinterpret_cast<UINT64>(entry.Data);

    return S_OK;
    /*   

    std::wstring postPolicyIdentity(pBindInfo->lpPostPolicyIdentity);

    const auto idx = postPolicyIdentity.find_first_of(L',');
    if (idx < 0 || idx > postPolicyIdentity.length())
        return COR_E_FILENOTFOUND;
        
    const auto requestedAssemblyName = postPolicyIdentity.substr(0, idx);

    static const std::tuple<const wchar_t* const, const WORD, const wchar_t* const, bool> embeddedAssemblies[] =
    {
        std::make_tuple(L"Ten18.Net",          IDR_TEN_18_ASSEMBLY_0, L"processorarchitecture=x86", true),
        std::make_tuple(L"Ten18.Interop",      IDR_TEN_18_ASSEMBLY_1, L"processorarchitecture=x86", true),
        std::make_tuple(L"Ten18.Interop.Impl", IDR_TEN_18_ASSEMBLY_2, L"processorarchitecture=x86", true),
        std::make_tuple(L"SlimMath",           IDR_TEN_18_ASSEMBLY_3, L"processorarchitecture=x86", true),
        std::make_tuple(L"AsyncCtpLibrary",    IDR_TEN_18_ASSEMBLY_4, L"processorarchitecture=msil", false),
    };

    for (int i = 0; i < ARRAYSIZE(embeddedAssemblies); ++i)
    {    
        if (requestedAssemblyName.length() != std::wcslen(std::get<0>(embeddedAssemblies[i])))
            continue;

        if (0 == _wcsnicmp(requestedAssemblyName.c_str(), std::get<0>(embeddedAssemblies[i]), requestedAssemblyName.length()))
        {
            if (std::wstring::npos == postPolicyIdentity.find(std::get<2>(embeddedAssemblies[i])))
                continue;

            const auto securityCookie = 0xBA5E1018;
            *pContext = securityCookie;

            if (i == 1)
                *ppStmAssemblyImage = new EmbeddedResourceStream(g_Ten18_Interop_Impl_dll, g_Ten18_Interop_Impl_dllSize);
            else
                *ppStmAssemblyImage = new EmbeddedResourceStream(std::get<1>(embeddedAssemblies[i]));

            *pAssemblyId = mUniqueAssemblyIds + i;

            if (std::get<3>(embeddedAssemblies[i]))
            {            
                const std::wstring dir(L"D://Projects//Code//Ten18//Code//obj//x86//Debug//");
                auto pdb = dir + std::get<0>(embeddedAssemblies[i]) + L".pdb";            
                Expect.HR = SHCreateStreamOnFileEx(pdb.c_str(), STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, nullptr, ppStmPDB);
            }

            return S_OK;
        }
    }
    
    return COR_E_FILENOTFOUND;
    
    */    
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
