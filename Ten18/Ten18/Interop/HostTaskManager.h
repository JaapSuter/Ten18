#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 {
    
    namespace Interop {

        class Host;

    class HostTaskManager : public IHostTaskManager
    {
    public:

        explicit HostTaskManager(Host& host);
        
        Ten18_QUERY_INTERFACE_BEGIN(IHostTaskManager)
            Ten18_QUERY_INTERFACE_IMPL(IHostTaskManager)
        Ten18_QUERY_INTERFACE_END()
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentTask(IHostTask **pTask);
        
        virtual HRESULT STDMETHODCALLTYPE CreateTask(DWORD dwStackSize, LPTHREAD_START_ROUTINE pStartAddress, PVOID pParameter, IHostTask **ppTask);
        
        virtual HRESULT STDMETHODCALLTYPE Sleep(DWORD dwMilliseconds, DWORD option);
        
        virtual HRESULT STDMETHODCALLTYPE SwitchToTask(DWORD option);
        
        virtual HRESULT STDMETHODCALLTYPE SetUILocale(LCID lcid);        
        virtual HRESULT STDMETHODCALLTYPE SetLocale(LCID lcid);
        
        virtual HRESULT STDMETHODCALLTYPE CallNeedsHostHook(SIZE_T target, BOOL *pbCallNeedsHostHook);
        
        virtual HRESULT STDMETHODCALLTYPE LeaveRuntime(SIZE_T target);        
        virtual HRESULT STDMETHODCALLTYPE EnterRuntime();
        
        virtual HRESULT STDMETHODCALLTYPE ReverseLeaveRuntime();        
        virtual HRESULT STDMETHODCALLTYPE ReverseEnterRuntime();
        
        virtual HRESULT STDMETHODCALLTYPE BeginDelayAbort();        
        virtual HRESULT STDMETHODCALLTYPE EndDelayAbort();
        
        virtual HRESULT STDMETHODCALLTYPE BeginThreadAffinity();        
        virtual HRESULT STDMETHODCALLTYPE EndThreadAffinity();
        
        virtual HRESULT STDMETHODCALLTYPE SetStackGuarantee(ULONG guarantee);        
        virtual HRESULT STDMETHODCALLTYPE GetStackGuarantee(ULONG *pGuarantee);        
        virtual HRESULT STDMETHODCALLTYPE SetCLRTaskManager(ICLRTaskManager *ppManager);

    private:

        HostTaskManager(const HostTaskManager&);
        HostTaskManager& operator = (const HostTaskManager&);

        Host* mHost;
    };
}}
