#pragma once
#include "Ten18/Graphics/Constants.h"
#include "Ten18/Graphics/Writer.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Image.h"

namespace Ten18 { namespace Graphics {

    class SwapChain;
        
    class Device
    {
    public:

        Device();
        ~Device();

        void Tick();
        
        void CreateDynamicTextures(int width, int height);
        void TickDynamicTextures(Image::Ptr&& img);
        void TickFrameRate();

    private:
    public: // Todo, April 2011, Jaap Suter needs to get things done.

        void InitializeShaders(const wchar_t* vsid, const wchar_t* psid, COM::COMPtr<ID3D11VertexShader>& vs, COM::COMPtr<ID3D11PixelShader>& ps);

        friend class SwapChain;
        
        Device(const Device&);
        Device& operator = (const Device&);

        bool                        mReposeInducedDwmFlushReq;

        COM::COMPtr<IDXGIDevice1>               mDXGIDevice1;
        COM::COMPtr<IDXGIAdapter1>              mDXGIAdapter1;
        COM::COMPtr<IDXGIFactory1>              mDXGIFactory1;
        COM::COMPtr<ID3D11Device>               mD3D11Device;
        COM::COMPtr<ID3D11Debug>                mD3D11Debug;
        COM::COMPtr<ID3D11DeviceContext>        mImmediateContext;

        Writer                                  mWriter;

        static const int                        DynamicTextureCount = 3;        
        COM::COMPtr<ID3D11Texture2D>            mDynamicTextures[DynamicTextureCount];
        COM::COMPtr<ID3D11ShaderResourceView>   mDynamicShaderResourceViews[DynamicTextureCount];
        int                                     mDynamicTexturesLRU;

        COM::COMPtr<ID3D11VertexShader>         mCaptureVertexShader;
        COM::COMPtr<ID3D11PixelShader>          mCapturePixelShader;
        COM::COMPtr<ID3D11VertexShader>         mGrayCodeVertexShader;
        COM::COMPtr<ID3D11PixelShader>          mGrayCodePixelShader;
        COM::COMPtr<ID3D11InputLayout>          mVertexLayout;
        COM::COMPtr<ID3D11Buffer>               mVertexBuffer;
        COM::COMPtr<ID3D11Buffer>               mConstantBuffer;
        COM::COMPtr<ID3D11ShaderResourceView>   mTextureRV;
        COM::COMPtr<ID3D11SamplerState>         mSamplerLinear;
        COM::COMPtr<ID3D11BlendState>           mAlphaBlendState;
        COM::COMPtr<ID3D11RasterizerState>      mRasterizerState;

        ID3D11ShaderResourceView*               mFpsTex;
        std::vector<SwapChain*>                 mSwapChains;
    
    public:
        // Todo, Jaap Suter, April 2011
        std::vector<std::function<void ()>>     mTickSignalRequests;
    };

}}
