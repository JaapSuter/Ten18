#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Interop/IAppDomainManagerEx.h"

namespace Ten18 {
    
    namespace Interop {

        class Host;

    class HostGCManager : public IHostGCManager
    {
    public:

        explicit HostGCManager(Host& host);
        
        Ten18_QUERY_INTERFACE_BEGIN(IHostGCManager)
            Ten18_QUERY_INTERFACE_IMPL(IHostGCManager)
        Ten18_QUERY_INTERFACE_END()
        
        virtual HRESULT STDMETHODCALLTYPE ThreadIsBlockingForSuspension();
        virtual HRESULT STDMETHODCALLTYPE SuspensionStarting();        
        virtual HRESULT STDMETHODCALLTYPE SuspensionEnding(DWORD Generation);

    private:

        HostGCManager(const HostGCManager&);
        HostGCManager& operator = (const HostGCManager&);

        Host* mHost;
    };
}}
