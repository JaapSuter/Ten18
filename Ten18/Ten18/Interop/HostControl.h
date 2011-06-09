#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 {
    
    namespace Interop {

        class Host;

    class HostControl : public IHostControl
    {
    public:
        explicit HostControl(Host& host);

        Ten18_QUERY_INTERFACE_BEGIN(IHostControl)
            Ten18_QUERY_INTERFACE_IMPL(IHostControl)
        Ten18_QUERY_INTERFACE_END()
        
        virtual HRESULT STDMETHODCALLTYPE GetHostManager(REFIID riid, void** ppObject);
        virtual HRESULT STDMETHODCALLTYPE SetAppDomainManager(DWORD dwAppDomainID, IUnknown* pUnkAppDomainManager);

    private:
        
        HostControl(const HostControl&);
        HostControl& operator = (const HostControl&);
        
        Host* mHost;
    };
}}
