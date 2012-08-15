#include "Ten18/PCH.h"
#include "Ten18/Expect.h"
#include "Ten18/OnExit.h"
#include "Ten18/Timer.h"
#include "Ten18/Tracer.h"
#include "Ten18/Capture/MediaFoundationCapture.h"
#include "Ten18/Graphics/GraphicsDevice.h"
#include "Ten18/Window.h"
#include "Ten18/Format.h"

using namespace Ten18;

int __stdcall wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    Ten18_EXPECT.True = HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    Ten18_EXPECT.True = SetDllDirectory(L"");
    Ten18_EXPECT.True = dx::XMVerifyCPUSupport();
    Ten18_EXPECT.HR = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);

    Timer::Initialize();
    Memory::Initialize();

    // std::tr2::sys::wpath pictureLibraryPath;
    // {
    //     PWSTR rawPath = nullptr;
    //     OnExit oe([&]{ if (rawPath) CoTaskMemFree(rawPath); rawPath = nullptr; });
    //     Ten18_EXPECT.HR = SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &rawPath);
    //     pictureLibraryPath = rawPath;
    // }
    // 
    // for (auto iter = std::tr2::sys::wrecursive_directory_iterator(pictureLibraryPath); iter != decltype(iter)(); ++iter)
    // {
    //     DebugOut("%0", iter->path().string().c_str());
    // }

    Graphics::GraphicsDevice::Initialize();
    // Capture::MediaFoundationCapture::Initialize();

    Window window(L"Ten18");
    Graphics::SwapChain swapChain(window.Handle());
    
    MSG msg = {};
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Graphics::GraphicsDevice::Instance().Render();
        }
    }

    PostQuitMessage(static_cast<int>(msg.wParam));

    // Capture::MediaFoundationCapture::Shutdown();
    Graphics::GraphicsDevice::Shutdown();

    Memory::Shutdown();

    CoUninitialize();
}
