#include "Ten18/PCH.h"
#include "Ten18/Interop/HostTaskManager.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::Interop;
using namespace Ten18::COM;

HostTaskManager::HostTaskManager(Host& host)
    : mHost(&host)
{}

HRESULT STDMETHODCALLTYPE HostTaskManager::GetCurrentTask(IHostTask **pTask)
{
    UNREFERENCED_PARAMETER(pTask);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::CreateTask(DWORD dwStackSize, LPTHREAD_START_ROUTINE pStartAddress, PVOID pParameter, IHostTask **ppTask)
{
    UNREFERENCED_PARAMETER(dwStackSize);
    UNREFERENCED_PARAMETER(pStartAddress);
    UNREFERENCED_PARAMETER(pParameter);
    UNREFERENCED_PARAMETER(ppTask);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::Sleep(DWORD dwMilliseconds, DWORD option)
{
    UNREFERENCED_PARAMETER(dwMilliseconds);
    UNREFERENCED_PARAMETER(option);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::SwitchToTask(DWORD option)
{
    UNREFERENCED_PARAMETER(option);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::SetUILocale(LCID lcid)
{
    UNREFERENCED_PARAMETER(lcid);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::SetLocale(LCID lcid)
{
    UNREFERENCED_PARAMETER(lcid);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::CallNeedsHostHook(SIZE_T target, BOOL *pbCallNeedsHostHook)
{
    UNREFERENCED_PARAMETER(target);
    UNREFERENCED_PARAMETER(pbCallNeedsHostHook);
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::LeaveRuntime(SIZE_T target)
{
    UNREFERENCED_PARAMETER(target);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::EnterRuntime()
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::ReverseLeaveRuntime()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::ReverseEnterRuntime()
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::BeginDelayAbort()
{   
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::EndDelayAbort()
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::BeginThreadAffinity()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::EndThreadAffinity()
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE HostTaskManager::SetStackGuarantee(ULONG guarantee)    
{
    UNREFERENCED_PARAMETER(guarantee);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::GetStackGuarantee(ULONG *pGuarantee)    
{
    UNREFERENCED_PARAMETER(pGuarantee);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostTaskManager::SetCLRTaskManager(ICLRTaskManager *ppManager)
{
    UNREFERENCED_PARAMETER(ppManager);
    return S_OK;
}