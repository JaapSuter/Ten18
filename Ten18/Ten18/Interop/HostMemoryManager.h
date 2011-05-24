#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Interop/IAppDomainManagerEx.h"

namespace Ten18 {
    
    namespace Interop {

        class Host;

    class HostMemoryManager : public IHostMemoryManager 
    {
    public:

        explicit HostMemoryManager(Host& host)
            : mHost(&host) {}
        
        Ten18_QUERY_INTERFACE_BEGIN(IHostMemoryManager)
            Ten18_QUERY_INTERFACE_IMPL(IHostMemoryManager)
        Ten18_QUERY_INTERFACE_END()
                
        virtual HRESULT STDMETHODCALLTYPE CreateMalloc(DWORD dwMallocType, IHostMalloc **ppMalloc);

        virtual HRESULT STDMETHODCALLTYPE VirtualAlloc( 
            void *pAddress,
            SIZE_T dwSize,
            DWORD flAllocationType,
            DWORD flProtect,
            EMemoryCriticalLevel eCriticalLevel,
            void **ppMem);

        virtual HRESULT STDMETHODCALLTYPE VirtualFree( 
            LPVOID lpAddress,
            SIZE_T dwSize,
            DWORD dwFreeType);

        virtual HRESULT STDMETHODCALLTYPE VirtualQuery( 
            void *lpAddress,
            void *lpBuffer,
            SIZE_T dwLength,
            SIZE_T *pResult);

        virtual HRESULT STDMETHODCALLTYPE VirtualProtect( 
            void *lpAddress,
            SIZE_T dwSize,
            DWORD flNewProtect,
            DWORD *pflOldProtect);

        virtual HRESULT STDMETHODCALLTYPE GetMemoryLoad( 
            DWORD *pMemoryLoad,
            SIZE_T *pAvailableBytes);

        virtual HRESULT STDMETHODCALLTYPE RegisterMemoryNotificationCallback( 
            ICLRMemoryNotificationCallback *pCallback);

        virtual HRESULT STDMETHODCALLTYPE NeedsVirtualAddressSpace( 
            LPVOID startAddress,
            SIZE_T size);

        virtual HRESULT STDMETHODCALLTYPE AcquiredVirtualAddressSpace( 
            LPVOID startAddress,
            SIZE_T size);

        virtual HRESULT STDMETHODCALLTYPE ReleasedVirtualAddressSpace( 
            LPVOID startAddress);

    private:

        HostMemoryManager(const HostMemoryManager&);
        HostMemoryManager& operator = (const HostMemoryManager&);
        
        Host* mHost;
        std::vector<HostMalloc> mHostMallocs;        
    };
}}
