#pragma once
#include "Ten18/Graphics/Constants.h"
#include "Ten18/COM/COMPtr.h"

namespace Ten18 { 
    class Window;
    
    namespace Graphics {

        class Device;

    class SwapChain
    {
    public:
        
        explicit SwapChain(const std::function<void ()>& tick, Device& device, Window& window);
        void Tick();

        Window& GetWindow() const { return mWindow; }
        
    private:
        
        void Repose();

        SwapChain(const SwapChain&);
        SwapChain& operator = (const SwapChain&);

        Window&             mWindow;
        Device&             mDevice;

        std::function<void ()>      mTick;

        bool                        mClientEmpty;
        RECT                        mClientRect;
        POINT                       mClientPos;

        COM::COMPtr<IDXGISwapChain>         mSwapChain;
        COM::COMPtr<ID3D11RenderTargetView> mRenderTargetView;

        Constants                   mConstants;        
    };
}}
