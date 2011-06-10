#include "Ten18/PCH.h"
#include "Ten18/Expect.h"
#include "Ten18/OnExit.h"
#include "Ten18/Timer.h"
#include "Ten18/Tracer.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Capture/MediaFoundationCapture.h"
#include "Ten18/Graphics/GraphicsDevice.h"

using namespace Ten18;
using namespace Ten18::Interop;

int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    Expect.True = HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    Expect.True = SetDllDirectory(L"");
    Expect.True = XMVerifyCPUSupport();
    Expect.HR = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

    Timer::Initialize();
    Memory::Initialize();
    Graphics::GraphicsDevice::Initialize();
    Capture::MediaFoundationCapture::Initialize();    

    Host host([]
    {
        Capture::MediaFoundationCapture::Shutdown();
        Graphics::GraphicsDevice::Shutdown();
        Memory::Shutdown();
    });

    host.Run();
    
    // We don't call CoUninitialize because the CLR host is never returning anyway, and while
    // multiple matching CoInit and CoUninit calls should balance out, I'm just gonnna throw this cargo cult 
    // voodoo out the window, sacrifice a chicken, and focus on things that I care more about....
}

