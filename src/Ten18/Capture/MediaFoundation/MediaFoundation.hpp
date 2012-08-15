#ifndef PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_FOUNDATION_HPP
#define PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_FOUNDATION_HPP

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

    class MediaFoundation
	{
    public:
        
        static void Startup();
        static void Shutdown();
        
        static void GetDeviceNotifications(std::intptr_t hwnd);
        static void NoLongerGetDeviceNotifications();
    };
}}}

#endif
