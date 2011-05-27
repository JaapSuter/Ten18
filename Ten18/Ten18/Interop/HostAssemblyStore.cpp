#include "Ten18/PCH.h"
#include "Ten18/Interop/HostAssemblyStore.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Interop/PatchTable.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Content/Index.h"

#include "Ten18/Input/Input.h"
#include "Ten18/Window.h"
#include "Ten18/Interop/NativeFactory.h"

using namespace Ten18;
using namespace Ten18::Interop;
using namespace Ten18::COM;

static bool FileExists(const wchar_t *fileName)
{
    return INVALID_FILE_ATTRIBUTES != GetFileAttributes(fileName);
}

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

    auto entry = Content::Index::TryGet(pBindInfo->lpPostPolicyIdentity);
    if (nullptr == entry)
        return COR_E_FILENOTFOUND;
    
    if (entry->NeedsInteropPatch)
        PatchTable::Update(const_cast<char*>(entry->Data), entry->Size);
    
    const auto securityCookie = 0xBA5E1018;
    *pContext = securityCookie;
    *ppStmAssemblyImage = new EmbeddedResourceStream(entry->Data, entry->Size);
    *pAssemblyId = reinterpret_cast<UINT64>(entry->Data);

    std::wstring postPolicyIdentity(pBindInfo->lpPostPolicyIdentity);
    const auto idx = postPolicyIdentity.find_first_of(L',');
    if (idx != std::wstring::npos)
    {
        const auto requestedAssemblyName = postPolicyIdentity.substr(0, idx);
        const std::wstring dir(L"..\\..\\..\\obj\\x86\\Debug\\");
        auto pdb = dir + requestedAssemblyName + L".pdb";
        if (FileExists(pdb.c_str()))
        {
            Expect.HR = SHCreateStreamOnFileEx(pdb.c_str(), STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, nullptr, ppStmPDB);
        }
    }

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
