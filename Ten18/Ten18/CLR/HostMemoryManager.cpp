#include "Ten18/PCH.h"
#include "Ten18/CLR/HostMemoryManager.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::CLR;
using namespace Ten18::COM;

HRESULT STDMETHODCALLTYPE HostMemoryManager::CreateMalloc(DWORD dwMallocType, IHostMalloc **ppMalloc)
{
    Ten18_ASSERT(ppMalloc);
    if (!ppMalloc)
        return E_FAIL;

    mHostMallocs.emplace_back(HostMalloc(dwMallocType));
    *ppMalloc = &mHostMallocs.back();
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostMemoryManager::VirtualAlloc(void *pAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect,
    EMemoryCriticalLevel, void **ppMem)
{ 
    *ppMem = ::VirtualAlloc(pAddress, dwSize, flAllocationType, flProtect);
    return ppMem ? S_OK : E_OUTOFMEMORY;
}
        
HRESULT STDMETHODCALLTYPE HostMemoryManager::VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
    // Todo, Jaap Suter, April 2011, if memory not ours, then return HOST_E_INVALIDOPERATION, see MSDN.
    HopeFor.NotZero = ::VirtualFree(lpAddress, dwSize, dwFreeType);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostMemoryManager::VirtualQuery(void *lpAddress, void *lpBuffer, SIZE_T dwLength, SIZE_T *pResult)
{
    if (nullptr == pResult)
        return E_FAIL;

    *pResult = HopeFor.NotZero = ::VirtualQuery(lpAddress, static_cast<MEMORY_BASIC_INFORMATION*>(lpBuffer), dwLength);

    return 0 == *pResult ? E_FAIL : S_OK;
}

HRESULT STDMETHODCALLTYPE HostMemoryManager::VirtualProtect(void *lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD *pflOldProtect)
{
    auto ret = HopeFor.NotZero = ::VirtualProtect(lpAddress, dwSize, flNewProtect, pflOldProtect);
    return ret ? S_OK : E_FAIL;
}
        
HRESULT STDMETHODCALLTYPE HostMemoryManager::GetMemoryLoad(DWORD *pMemoryLoad, SIZE_T *pAvailableBytes)
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
        
HRESULT STDMETHODCALLTYPE HostMemoryManager::RegisterMemoryNotificationCallback(ICLRMemoryNotificationCallback*) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostMemoryManager::NeedsVirtualAddressSpace(LPVOID, SIZE_T) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostMemoryManager::AcquiredVirtualAddressSpace(LPVOID, SIZE_T) { return S_OK; }
HRESULT STDMETHODCALLTYPE HostMemoryManager::ReleasedVirtualAddressSpace(LPVOID) { return S_OK; }
