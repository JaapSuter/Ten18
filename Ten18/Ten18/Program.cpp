/*
    #include "Ten18/PCH.h"
    #include "Ten18/Expect.h"
    #include "Ten18/COM/EmbeddedResourceStream.h"
    #include "Ten18/Resources/Resources.h"
    #include "Ten18/COM/COMPtr.h"
    #include "Ten18/Interop/Host.h"
    #include "Ten18/Window.h"
    #include "Ten18/Graphics/GraphicsDevice.h"
    #include "Ten18/Graphics/Display.h"
    #include "Ten18/Graphics/SwapChain.h"
    #include "Ten18/Assert.h"
    #include "Ten18/Tracer.h"

    Window first(L"Ten18 Calibration");
    Window second(L"Ten18 Camera");

    GraphicsDevice device;
    
    SwapChain primary([&]
    {
        device.mImmediateContext->PSSetShader(device.mGrayCodePixelShader.Raw(), nullptr, 0);
        device.mImmediateContext->VSSetShader(device.mGrayCodeVertexShader.Raw(), nullptr, 0);

    }, device, first);

    

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
*/
