#pragma once
#include "Ten18/Graphics/Constants.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Memory.h"

namespace Ten18 { 
    class Window;
    
    namespace Graphics {

        class GraphicsDevice;

    class SwapChain
    {
        Ten18_NO_DEFAULT_OPERATOR_NEW_DELETE
    public:
        
        explicit SwapChain(HWND hwnd);
        void Tick();

    private:
        
        void Repose();

        SwapChain(const SwapChain&);
        SwapChain& operator = (const SwapChain&);

        HWND                mHwnd;
        GraphicsDevice&     mDevice;

        bool                mClientEmpty;
        RECT                mClientRect;
        POINT               mClientPos;

        COM::COMPtr<IDXGISwapChain>         mSwapChain;
        COM::COMPtr<ID3D11RenderTargetView> mRenderTargetView;

        Constants           mConstants;        
    };
}}
