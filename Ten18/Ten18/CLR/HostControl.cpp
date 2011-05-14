#include "Ten18/PCH.h"
#include "Ten18/CLR/HostControl.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Hosting/IManagedServiceProvider.Generated.h"

using namespace Ten18;
using namespace Ten18::CLR;
using namespace Ten18::COM;
using namespace Ten18::Hosting;

HostControl::HostControl() :
    mUniqueAssemblyIds(1018),
    mManagedServiceProvider()
{}

HRESULT STDMETHODCALLTYPE HostControl::GetHostManager(REFIID riid, void** ppObject)
{
    if (riid == IID_IHostAssemblyManager)
        *ppObject = static_cast<IHostAssemblyManager*>(this);
    else if (riid == IID_IHostMemoryManager)
        *ppObject = static_cast<IHostMemoryManager*>(this);
    else if (riid == IID_IHostAssemblyStore)
        *ppObject = static_cast<IHostMemoryManager*>(this);
    else
        if (ppObject != nullptr)
            *ppObject = nullptr;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostControl::SetAppDomainManager(DWORD dwAppDomainID, IUnknown* appDomainManagerAsUnknown)
{
    UNREFERENCED_PARAMETER(dwAppDomainID);
    Ten18_ASSERT(dwAppDomainID == 1);
    Expect.HR = appDomainManagerAsUnknown->QueryInterface(__uuidof(mManagedServiceProvider), reinterpret_cast<void**>(&mManagedServiceProvider));
    Ten18_ASSERT(mManagedServiceProvider != nullptr);
    return S_OK;
}

void HostControl::Tick()
{
}

HRESULT STDMETHODCALLTYPE HostControl::GetNonHostStoreAssemblies(ICLRAssemblyReferenceList **ppReferenceList)
{   
    ppReferenceList = nullptr;        
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostControl::GetAssemblyStore(IHostAssemblyStore **ppAssemblyStore)
{
    *ppAssemblyStore = static_cast<IHostAssemblyStore*>(this);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostControl::ProvideAssembly(
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

    std::wstring postPolicyIdentity(pBindInfo->lpPostPolicyIdentity);

    const auto idx = postPolicyIdentity.find_first_of(L',');
    if (idx < 0 || idx > postPolicyIdentity.length())
        return COR_E_FILENOTFOUND;
        
    const auto requestedAssemblyName = postPolicyIdentity.substr(0, idx);

    static const std::tuple<const wchar_t* const, const WORD, const wchar_t* const> embeddedAssemblies[] =
    {
        std::make_tuple(L"Ten18.Net",         IDR_TEN_18_NET_ASSEMBLY, L"processorarchitecture=x86"),
        std::make_tuple(L"Ten18.Net.Interop", IDR_TEN_18_NET_INTEROP_ASSEMBLY, L"processorarchitecture=x86"),
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
            *ppStmAssemblyImage = new EmbeddedResourceStream(std::get<1>(embeddedAssemblies[i]));
            *pAssemblyId = mUniqueAssemblyIds + i;
            
            const std::wstring dir(L"D://Projects//Code//Ten18//Code//obj//Win32//Debug//");
            auto pdb = dir + std::get<0>(embeddedAssemblies[i]) + L".pdb";            
            Expect.HR = SHCreateStreamOnFileEx(pdb.c_str(), STGM_READ | STGM_SHARE_DENY_NONE, 0, FALSE, nullptr, ppStmPDB);

            return S_OK;
        }
    }
    
    return COR_E_FILENOTFOUND;
}
        
HRESULT STDMETHODCALLTYPE HostControl::ProvideModule( 
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
    
HRESULT STDMETHODCALLTYPE HostControl::CreateMalloc(DWORD dwMallocType, IHostMalloc **ppMalloc)
{
    Ten18_ASSERT(ppMalloc);
    if (!ppMalloc)
        return E_FAIL;

    mHostMallocs.emplace_back(HostMalloc(dwMallocType));
    *ppMalloc = &mHostMallocs.back();
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostControl::VirtualAlloc(void *pAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect,
    EMemoryCriticalLevel, void **ppMem)
{ 
    *ppMem = ::VirtualAlloc(pAddress, dwSize, flAllocationType, flProtect);
    return ppMem ? S_OK : E_OUTOFMEMORY;
}
        
HRESULT STDMETHODCALLTYPE HostControl::VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
    // Todo, Jaap Suter, April 2011, if memory not ours, then return HOST_E_INVALIDOPERATION, see MSDN.
    HopeFor.NotZero = ::VirtualFree(lpAddress, dwSize, dwFreeType);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostControl::VirtualQuery(void *lpAddress, void *lpBuffer, SIZE_T dwLength, SIZE_T *pResult)
{
    if (nullptr == pResult)
        return E_FAIL;

    *pResult = HopeFor.NotZero = ::VirtualQuery(lpAddress, static_cast<MEMORY_BASIC_INFORMATION*>(lpBuffer), dwLength);

    return 0 == *pResult ? E_FAIL : S_OK;
}

HRESULT STDMETHODCALLTYPE HostControl::VirtualProtect(void *lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD *pflOldProtect)
{
    auto ret = HopeFor.NotZero = ::VirtualProtect(lpAddress, dwSize, flNewProtect, pflOldProtect);
    return ret ? S_OK : E_FAIL;
}
        
HRESULT STDMETHODCALLTYPE HostControl::GetMemoryLoad(DWORD *pMemoryLoad, SIZE_T *pAvailableBytes)
{ 
    MEMORYSTATUSEX ms = {};
    ms.dwLength = sizeof(ms);
    auto hr = GlobalMemoryStatusEx(&ms);
    if (SUCCEEDED(hr))
    {
        const auto conservativePercentage = 40.0 / 100.0;
        *pAvailableBytes = static_cast<SIZE_T>(ms.ullAvailVirtual * conservativePercentage);
        *pMemoryLoad = ms.dwMemoryLoad;
        return S_OK;
    }
    else return HOST_E_CLRNOTAVAILABLE;
}
        
HRESULT STDMETHODCALLTYPE HostControl::RegisterMemoryNotificationCallback(ICLRMemoryNotificationCallback*) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostControl::NeedsVirtualAddressSpace(LPVOID, SIZE_T) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostControl::AcquiredVirtualAddressSpace(LPVOID, SIZE_T) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostControl::ReleasedVirtualAddressSpace(LPVOID) { return S_OK; }
