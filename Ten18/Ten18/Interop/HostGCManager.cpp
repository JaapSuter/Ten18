#include "Ten18/PCH.h"
#include "Ten18/Interop/HostGCManager.h"
#include "Ten18/Interop/HostMalloc.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::Interop;
using namespace Ten18::COM;

HostGCManager::HostGCManager(Host& host)
    : mHost(&host)
{}

HRESULT STDMETHODCALLTYPE HostGCManager::ThreadIsBlockingForSuspension()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostGCManager::SuspensionStarting()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostGCManager::SuspensionEnding(DWORD generation)
{
    UNREFERENCED_PARAMETER(generation);
    return S_OK;
}
