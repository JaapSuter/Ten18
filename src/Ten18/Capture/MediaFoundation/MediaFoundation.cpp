#include "Pentacorn/Precompiled.Cpp.hpp"
#include "Pentacorn/Error.hpp"
#include "Pentacorn/Capture/MediaFoundation/MediaFoundation.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

    namespace {
        HDEVNOTIFY deviceNotifyHandle = nullptr;
        
        bool TryDeviceNotifierGrab()
        {
            HDEVNOTIFY dummyGrabToken = reinterpret_cast<HDEVNOTIFY>(-1);    
            return nullptr == InterlockedCompareExchangePointer(&deviceNotifyHandle, dummyGrabToken, nullptr);                
        }
    }

void MediaFoundation::Startup()
{
    Error = MFStartup(MF_VERSION, MFSTARTUP_LITE);
}

void MediaFoundation::Shutdown()
{
    Error = MFShutdown();
}

void MediaFoundation::GetDeviceNotifications(std::intptr_t hwnd)
{
	static_assert(sizeof(std::intptr_t) == sizeof(HWND), "Unexpected sizeof(HWND)");

    if (!TryDeviceNotifierGrab())
        throw std::exception("Currently only a single window can register for device notifications, sorry.");

    DEV_BROADCAST_DEVICEINTERFACE di = {};
    di.dbcc_size = sizeof(di);
    di.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
    di.dbcc_classguid  = KSCATEGORY_CAPTURE; 

    deviceNotifyHandle = RegisterDeviceNotification(reinterpret_cast<HWND>(hwnd), &di, DEVICE_NOTIFY_WINDOW_HANDLE);
    if (deviceNotifyHandle == nullptr)
        throw std::exception("RegisterDeviceNotification");
}

void MediaFoundation::NoLongerGetDeviceNotifications()
{
    if (deviceNotifyHandle != nullptr)
        if (!::UnregisterDeviceNotification(deviceNotifyHandle))
            throw std::exception("UnregisterDeviceNotification");
    deviceNotifyHandle = nullptr;
}

}}}
