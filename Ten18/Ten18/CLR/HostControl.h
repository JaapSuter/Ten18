#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/CLR/HostMalloc.h"

namespace Ten18 {
    
    struct IManagedServiceProvider;

    namespace CLR {

    class HostControl : public IHostControl, IHostAssemblyManager, IHostAssemblyStore, IHostMemoryManager 
    {
    public:

        void Tick();
        IManagedServiceProvider* GetManagedServiceProvider() { return mManagedServiceProvider; }
        HostControl();
        
        Ten18_QUERY_INTERFACE_BEGIN(IHostControl)
            Ten18_QUERY_INTERFACE_IMPL(IHostControl)
            Ten18_QUERY_INTERFACE_IMPL(IHostAssemblyManager)
            Ten18_QUERY_INTERFACE_IMPL(IHostAssemblyStore)
            Ten18_QUERY_INTERFACE_IMPL(IHostMemoryManager)
        Ten18_QUERY_INTERFACE_END()
        
        virtual HRESULT STDMETHODCALLTYPE GetHostManager(REFIID riid, void** ppObject);
        virtual HRESULT STDMETHODCALLTYPE SetAppDomainManager(DWORD dwAppDomainID, IUnknown* pUnkAppDomainManager);
        virtual HRESULT STDMETHODCALLTYPE GetNonHostStoreAssemblies(ICLRAssemblyReferenceList **ppReferenceList);
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyStore(IHostAssemblyStore **ppAssemblyStore);
        virtual HRESULT STDMETHODCALLTYPE ProvideAssembly(
            AssemblyBindInfo *pBindInfo,
            UINT64 *pAssemblyId,
            UINT64 *pContext,
            IStream **ppStmAssemblyImage,
            IStream **ppStmPDB);

        virtual HRESULT STDMETHODCALLTYPE ProvideModule( 
            ModuleBindInfo *pBindInfo,
            DWORD *pdwModuleId,
            IStream **ppStmModuleImage,
            IStream **ppStmPDB);

        virtual HRESULT STDMETHODCALLTYPE CreateMalloc( 
            DWORD dwMallocType,
            IHostMalloc **ppMalloc);

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

        IStream* ExtractResource(WORD resourceID);

        HostControl(const HostControl&);
        HostControl& operator = (const HostControl&);
        
        IManagedServiceProvider* mManagedServiceProvider;
        ULONG mUniqueAssemblyIds;
        std::vector<HostMalloc> mHostMallocs;
    };
}}
