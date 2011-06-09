#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 {
    
    namespace Interop {

        class Host;

    class HostAssemblyManager : public IHostAssemblyManager
    {
    public:

        explicit HostAssemblyManager(Host& host);
        
        Ten18_QUERY_INTERFACE_BEGIN(IHostAssemblyManager)
            Ten18_QUERY_INTERFACE_IMPL(IHostAssemblyManager)
        Ten18_QUERY_INTERFACE_END()
        
        virtual HRESULT STDMETHODCALLTYPE GetNonHostStoreAssemblies(ICLRAssemblyReferenceList **ppReferenceList);
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyStore(IHostAssemblyStore **ppAssemblyStore);

    private:

        HostAssemblyManager(const HostAssemblyManager&);
        HostAssemblyManager& operator = (const HostAssemblyManager&);

        Host* mHost;
    };
}}
