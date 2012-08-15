#pragma once

#include "Ten18/Graphics/Constants.h"
#include "Ten18/Graphics/Writer.h"
#include "Ten18/Graphics/Image.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Memory.h"

namespace Ten18 { namespace Graphics {

    class SwapChain;    
        
    class GraphicsDevice
    {
        Ten18_CUSTOM_OPERATOR_NEW_DELETE
    public:

        static void* New();

        static void Initialize();
        static void Shutdown();
        static GraphicsDevice& Instance();

        void Render();
        
        void CreateDynamicTextures(int width, int height);
        void TickDynamicTextures(Image::Ptr&& img);
        void TickFrameRate();

        // #include "Ten18/Graphics/GraphicsDevice.Generated.h"

    private:

        GraphicsDevice();
        ~GraphicsDevice();
        
    public: // Todo, April 2011, Jaap Suter needs to get things done.

        void InitializeShaders();
        void CreateInputLayoutDescFromVertexShaderSignature(const void* data, SIZE_T size, ID3D11InputLayout** pInputLayout);

        friend class SwapChain;
        
        GraphicsDevice(const GraphicsDevice&);
        GraphicsDevice& operator = (const GraphicsDevice&);

        bool                        mReposeInducedDwmFlushReq;

        COM::COMPtr<IDXGIDevice2>               mDXGIDevice2;
        COM::COMPtr<IDXGIAdapter2>              mDXGIAdapter2;
        COM::COMPtr<IDXGIFactory2>              mDXGIFactory2;
        COM::COMPtr<ID3D11Device1>              mD3D11Device1;
        COM::COMPtr<ID3D11Debug>                mD3D11Debug;
        COM::COMPtr<ID3D11DeviceContext>        mImmediateContext;

        Writer                                  mWriter;

        static const int                        DynamicTextureCount = 3;        
        COM::COMPtr<ID3D11Texture2D>            mDynamicTextures[DynamicTextureCount];
        COM::COMPtr<ID3D11ShaderResourceView>   mDynamicShaderResourceViews[DynamicTextureCount];
        int                                     mDynamicTexturesLRU;

        COM::COMPtr<ID3D11VertexShader>         mTwoDeeVertexShader;
        COM::COMPtr<ID3D11PixelShader>          mCapturePixelShader;
        COM::COMPtr<ID3D11PixelShader>          mGrayCodePixelShader;
        COM::COMPtr<ID3D11InputLayout>          mVertexLayout;
        COM::COMPtr<ID3D11Buffer>               mVertexBuffer;
        COM::COMPtr<ID3D11Buffer>               mConstantBuffer;
        COM::COMPtr<ID3D11ShaderResourceView>   mTextureRV;
        COM::COMPtr<ID3D11Texture2D>            mTexture2D;
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
