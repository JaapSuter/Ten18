#include "Ten18/PCH.h"
#include "Ten18/Program.h"
#include "Ten18/Expect.h"
#include "Ten18/Sandbox.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Capture/CLEyeCapture.h"
#include "Ten18/Capture/MediaFoundationCapture.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/Host.h"
#include "Ten18/Windows/Window.h"
#include "Ten18/Graphics/Device.h"
#include "Ten18/Graphics/Display.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Assert.h"
#include "Ten18/Tracer.h"

using namespace Ten18;
using namespace Ten18::COM;
using namespace Ten18::Windows;
using namespace Ten18::Graphics;
using namespace Ten18::Capture;

int Ten18::Program(HINSTANCE, int nCmdShow, Ten18::Interop::Host* host)
{   
    Window first(L"Ten18 Calibration");
    Window second(L"Ten18 Camera");

    Device device;
    
    SwapChain primary([&]
    {
        device.mImmediateContext->PSSetShader(device.mGrayCodePixelShader.Raw(), nullptr, 0);
        device.mImmediateContext->VSSetShader(device.mGrayCodeVertexShader.Raw(), nullptr, 0);

    }, device, first);

    MediaFoundationCapture::Initialize();
    
    CaptureSource::Ptr capture;
    if (MediaFoundationCapture::Count() > 0)
        capture = MediaFoundationCapture::Get(0);
    else if (CLEyeCapture::Count() > 0)
        capture = CLEyeCapture::Get(0);

    device.CreateDynamicTextures(capture->Width(), capture->Height());
    
    SwapChain secondary([&]
    {
        capture->Tick();
        device.TickDynamicTextures(capture->Latest());
        
        device.mImmediateContext->PSSetShader(device.mCapturePixelShader.Raw(), nullptr, 0);
        device.mImmediateContext->VSSetShader(device.mCaptureVertexShader.Raw(), nullptr, 0);

    }, device, second);

    first.Show(nCmdShow);
    first.ToggleWindowedFullScreen();
    
    second.Show(nCmdShow);
    const RECT mon = { -900, 100, 800, 600 };
    second.Repose(mon, 800, 600);

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
            if (host != nullptr)
                host->Tick();
    
            device.Tick();
        }
    }

    PostQuitMessage(static_cast<int>(msg.wParam));
    return static_cast<int>(msg.wParam);
}
