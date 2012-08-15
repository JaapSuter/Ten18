#include "Ten18/PCH.h"
#include "Ten18/Graphics/GraphicsDevice.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Graphics/Util.h"
#include "Ten18/Graphics/Display.h"
#include "Ten18/Graphics/Vertex.h"
#include "Ten18/Timer.h"
#include "Ten18/Expect.h"
#include "Ten18/Scoped.h"
#include "Ten18/OnExit.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Graphics/TextureLoader.h"
#include "Ten18/Tracer.h"

using namespace Ten18;
using namespace Ten18::Graphics;
using namespace Ten18::COM;

namespace Ten18 { namespace Graphics { namespace Shaders {
    #include "Ten18/Graphics/Shaders/PSCapture.h"
    #include "Ten18/Graphics/Shaders/PSGrayCode.h"
    #include "Ten18/Graphics/Shaders/VSTwoDeeShader.h"
}}}

static GraphicsDevice* sGraphicsDevice = nullptr;

void GraphicsDevice::Initialize()
{
    Ten18_ASSERT(sGraphicsDevice == nullptr);
    sGraphicsDevice = Ten18_NEW GraphicsDevice();
}

void GraphicsDevice::Shutdown()
{
    Ten18_ASSERT(sGraphicsDevice != nullptr);
    delete sGraphicsDevice;
    sGraphicsDevice = nullptr;
}

GraphicsDevice& GraphicsDevice::Instance()
{
    Ten18_ASSERT(sGraphicsDevice != nullptr);
    return *sGraphicsDevice;
}

void* GraphicsDevice::New()
{    
    return &GraphicsDevice::Instance();
}

GraphicsDevice::GraphicsDevice() : 
    mReposeInducedDwmFlushReq(), mDynamicTexturesLRU(), mFpsTex()
{
    const UINT flags = 0 // D3D11_CREATE_DEVICE_VIDEO_SUPPORT
                     | D3D11_CREATE_DEVICE_BGRA_SUPPORT
                     | Ten18_IF_DEBUG_ELSE(D3D11_CREATE_DEVICE_DEBUG, 0);
    
    auto featureLevel = D3D_FEATURE_LEVEL();
    const auto driverType = D3D_DRIVER_TYPE_HARDWARE;
                       
    {
        COM::COMPtr<ID3D11Device> device;
        COM::COMPtr<ID3D11DeviceContext> immediateContext;
        Ten18_EXPECT.HR = D3D11CreateDevice(nullptr, driverType, nullptr,
                                    flags, nullptr, 0, D3D11_SDK_VERSION,
                                    device.AsTypedDoubleStar(), &featureLevel, immediateContext.AsTypedDoubleStar());
        
        device.QueryInto(mD3D11Device1);
        SetDebugName(mD3D11Device1, "D3D11Device1");

        immediateContext.QueryInto(mImmediateContext1);
        SetDebugName(mImmediateContext1, "ImmediateContext1");
    }

    if (flags & D3D11_CREATE_DEVICE_DEBUG)
        mD3D11Device1.QueryInto(mD3D11Debug);

    // COM::COMPtr<ID3D11VideoDevice> videoDevice;
    // mD3D11Device1.QueryInto(videoDevice);
    // const auto dxvaProfileCount = videoDevice->GetVideoDecoderProfileCount();
    // for (UINT idx = 0; idx < dxvaProfileCount; ++idx)
    // {
    //     GUID dxvaProfileGuid = {};
    //     Ten18_EXPECT.HR = videoDevice->GetVideoDecoderProfile(idx, &dxvaProfileGuid);
    //     DebugOut("DXVA Profile GUID: %0", GuidToString(dxvaProfileGuid));
    // }

    mD3D11Device1.QueryInto(mDXGIDevice2);
    Ten18_EXPECT.HR = mDXGIDevice2->GetParent(mDXGIAdapter2.GetUUID(), mDXGIAdapter2.AsVoidDoubleStar());
    Ten18_EXPECT.HR = mDXGIAdapter2->GetParent(mDXGIFactory2.GetUUID(), mDXGIFactory2.AsVoidDoubleStar());
    
    Ten18_EXPECT.HR = mDXGIDevice2->SetMaximumFrameLatency(0);

    // const float hsw = 2048.0f;
    // const float hsh = 1024.0f;
    // const float hso = 0;
    // const float tsw = 2048;
    // const float tsh = 1024;
    // Vertex vertices[] =
    // {
    //     { dx::XMFLOAT4(hso + 0,     0, 0.5f, 1.0f), dx::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), dx::XMFLOAT2( 0, 0 ) },
    //     { dx::XMFLOAT4(hso + hsw,   0, 0.5f, 1.0f), dx::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), dx::XMFLOAT2( hsw / tsw, 0 ) },
    //     { dx::XMFLOAT4(hso +   0, hsh, 0.5f, 1.0f), dx::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), dx::XMFLOAT2( 0, hsh / tsh ) },
    //     { dx::XMFLOAT4(hso + hsw, hsh, 0.5f, 1.0f), dx::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), dx::XMFLOAT2( hsw / tsw, hsh / tsh) },
    // };
    
    InitializeShaders();

    // D3D11_BUFFER_DESC vbd = {};
    // vbd.Usage = D3D11_USAGE_DEFAULT;
    // vbd.ByteWidth = sizeof(vertices);
    // vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // vbd.CPUAccessFlags = 0;
    // D3D11_SUBRESOURCE_DATA subResourceData = {};
    // subResourceData.pSysMem = vertices;
    // Ten18_EXPECT.HR = mD3D11Device1->CreateBuffer(&vbd, &subResourceData, mVertexBuffer.AsTypedDoubleStar());
    // SetDebugName(mVertexBuffer, "Vertex Buffer");

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(Constants);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;
    Ten18_EXPECT.HR = mD3D11Device1->CreateBuffer(&cbd, NULL, mConstantBuffer.AsTypedDoubleStar());
    SetDebugName(mConstantBuffer, "Constant Buffer");

    Ten18_EXPECT.HR = CreateWICTextureFromFile(mD3D11Device1.Raw(), nullptr, L"images/SquareTest.jpg", nullptr, mTextureRV.AsTypedDoubleStar());    
    
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;    
    Ten18_EXPECT.HR = mD3D11Device1->CreateSamplerState(&sampDesc, mSamplerLinear.AsTypedDoubleStar());
    SetDebugName(mSamplerLinear, "Sampler");

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;    
    blendDesc.RenderTarget[0].SrcBlendAlpha= D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha  = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;    
    Ten18_EXPECT.HR = mD3D11Device1->CreateBlendState(&blendDesc, mAlphaBlendState.AsTypedDoubleStar());
    SetDebugName(mAlphaBlendState, "AlphaBlend");

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = true;
    Ten18_EXPECT.HR = mD3D11Device1->CreateRasterizerState(&rasterDesc, mRasterizerState.AsTypedDoubleStar());
    SetDebugName(mRasterizerState, "Solid");

    // UINT stride = sizeof(Vertex);
    // UINT offset = 0;
    // mImmediateContext1->IASetVertexBuffers(0, 1, mVertexBuffer.AsUnsafeArrayOfOne(), &stride, &offset);
    // mImmediateContext1->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    // mImmediateContext1->IASetInputLayout(mVertexLayout.Raw());
    mImmediateContext1->IASetInputLayout(nullptr);
    mImmediateContext1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    mImmediateContext1->OMSetBlendState(mAlphaBlendState.Raw(), 0, UINT_MAX);
    mImmediateContext1->RSSetState(mRasterizerState.Raw());

    mImmediateContext1->PSSetShaderResources(0, 1, mTextureRV.AsUnsafeArrayOfOne());
    mImmediateContext1->PSSetSamplers(0, 1, mSamplerLinear.AsUnsafeArrayOfOne());

    mImmediateContext1->PSSetShader(mCapturePixelShader.Raw(), nullptr, 0);
    mImmediateContext1->PSSetConstantBuffers(0, 1, mConstantBuffer.AsUnsafeArrayOfOne());
        
    mImmediateContext1->VSSetShader(mTwoDeeVertexShader.Raw(), nullptr, 0);
    mImmediateContext1->VSSetConstantBuffers(0, 1, mConstantBuffer.AsUnsafeArrayOfOne());

    mWriter.Initialize(*mDXGIAdapter2, *mDXGIDevice2, *mD3D11Device1);
}

void GraphicsDevice::InitializeShaders()
{
    Ten18_EXPECT.HR = mD3D11Device1->CreateVertexShader(Shaders::VSTwoDeeShader, sizeof(Shaders::VSTwoDeeShader), nullptr, mTwoDeeVertexShader.AsTypedDoubleStar());
    SetDebugName(mTwoDeeVertexShader, "Two Dee Vertex Shader");
    
    if (!mVertexLayout)
    {
        // Todo, Jaap Suter, July 2012;
        // Ten18_EXPECT.HR = mD3D11Device1->CreateInputLayout(Vertex::Layout, Vertex::LayoutCount, Shaders::VSTwoDeeShader, sizeof(Shaders::VSTwoDeeShader), mVertexLayout.AsTypedDoubleStar());
        CreateInputLayoutDescFromVertexShaderSignature(Shaders::VSTwoDeeShader, sizeof(Shaders::VSTwoDeeShader), mVertexLayout.AsTypedDoubleStar());        
        SetDebugName(mVertexLayout, "Two Dee Vertex Layout");
    }
    
    
    Ten18_EXPECT.HR = mD3D11Device1->CreatePixelShader(Shaders::PSCapture, sizeof(Shaders::PSCapture), nullptr, mCapturePixelShader.AsTypedDoubleStar());
    SetDebugName(mCapturePixelShader, "Capture Pixel Shader");

    Ten18_EXPECT.HR = mD3D11Device1->CreatePixelShader(Shaders::PSGrayCode, sizeof(Shaders::PSGrayCode), nullptr, mGrayCodePixelShader.AsTypedDoubleStar());
    SetDebugName(mGrayCodePixelShader, "GrayCode Pixel Shader");    
}

GraphicsDevice::~GraphicsDevice()
{
    if (mImmediateContext1)
    {
        mImmediateContext1->ClearState();
        mImmediateContext1->Flush();
    }
}

void GraphicsDevice::CreateInputLayoutDescFromVertexShaderSignature(const void* data, SIZE_T size, ID3D11InputLayout** pInputLayout)
{
    // This function is graciously based on code found at http://takinginitiative.net/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/
    Ten18_ASSERT(pInputLayout && (*pInputLayout == nullptr));

	COM::COMPtr<ID3D11ShaderReflection> vsReflection;
    Ten18_EXPECT.HR = D3DReflect(data, size, IID_ID3D11ShaderReflection, vsReflection.AsVoidDoubleStar());
	
    D3D11_SHADER_DESC shaderDesc = {};
	Ten18_EXPECT.HR = vsReflection->GetDesc(&shaderDesc);
	
	UINT byteOffset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (auto i = 0u; i < shaderDesc.InputParameters; ++i)
	{
        D3D11_SIGNATURE_PARAMETER_DESC paramDesc = {};		
		Ten18_EXPECT.HR = vsReflection->GetInputParameterDesc(i, &paramDesc);

        D3D11_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		
        // Todo, Jaap Suter, July 2012;
        elementDesc.AlignedByteOffset = byteOffset;
        elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		
        elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if      (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			byteOffset += 4;
		}
		else if (paramDesc.Mask <= 3)
		{
			if      (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			byteOffset += 8;
		}
		else if (paramDesc.Mask <= 7)
		{
			if      (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			byteOffset += 12;
		}
		else if (paramDesc.Mask <= 15)
		{
			if      (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			byteOffset += 16;
		}
			
		inputLayoutDesc.push_back(elementDesc);
	}		

    Ten18_EXPECT.HR = this->mD3D11Device1->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), data, size, pInputLayout);
}

void GraphicsDevice::Render()
{
    TickFrameRate();
    
    std::for_each(mSwapChains.begin(), mSwapChains.end(), [] (SwapChain* sw) { sw->Tick(); });
    
    if (mReposeInducedDwmFlushReq)
        DwmFlush();
    mReposeInducedDwmFlushReq = false;

    std::for_each(std::begin(mTickSignalRequests), std::end(mTickSignalRequests), [] (const std::function<void ()>& func) { func(); });
}


void GraphicsDevice::CreateDynamicTextures(int width, int height)
{
    D3D11_TEXTURE2D_DESC td = {};    
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.Width = width;
    td.Height = height;    
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DYNAMIC;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    for (int i = 0; i < DynamicTextureCount; ++i)
        Ten18_EXPECT.HR = mD3D11Device1->CreateTexture2D(&td, nullptr, mDynamicTextures[i].AsTypedDoubleStar());

    for (int i = 0; i < DynamicTextureCount; ++i)
        Ten18_EXPECT.HR = mD3D11Device1->CreateShaderResourceView(mDynamicTextures[i].Raw(), nullptr, mDynamicShaderResourceViews[i].AsTypedDoubleStar());
    
    mDynamicTexturesLRU = 0;
}

void GraphicsDevice::TickDynamicTextures(Image::Ptr&& img)
{
    if (!img)
    {
        mImmediateContext1->PSSetShaderResources(0, 1, mDynamicShaderResourceViews[mDynamicTexturesLRU].AsUnsafeArrayOfOne());
        return;
    }

    Ten18_ASSERT(img->BytesPerPixel() == 4);
    
    mDynamicTexturesLRU = (mDynamicTexturesLRU + 1) % DynamicTextureCount;
    auto& lru = mDynamicTextures[mDynamicTexturesLRU];
    
    // Todo, Jaap Suter, April 2011: figure out why D3D11_MAP_FLAG_DO_NOT_WAIT doesn't jive with D3D11_MAP_WRITE_DISCARD here.
    D3D11_MAPPED_SUBRESOURCE msr = {};
    {
        auto hr = mImmediateContext1->Map(lru.Raw(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
        {
            DebugOut("DXGI_ERROR_WAS_STILL_DRAWING");
            return;
        }
        else Ten18_EXPECT.HR = hr;

        OnExit oe([&] { mImmediateContext1->Unmap(lru.Raw(), 0); });
    }

    if (static_cast<int>(msr.RowPitch) == img->RowPitch())
        memcpy_s(msr.pData, img->Size(), img->DataAs<void>(), img->Size());
    else for (int y = 0; y < img->Height(); ++y)
    {
        auto dst = static_cast<char*>(msr.pData) + y * msr.RowPitch;
        auto src = img->DataAs<char>() + y * img->RowPitch();
        auto len = img->Width() * img->BytesPerPixel();
        memcpy_s(dst, len, src, len);
    }

    mImmediateContext1->PSSetShaderResources(0, 1, mDynamicShaderResourceViews[mDynamicTexturesLRU].AsUnsafeArrayOfOne());
}

void GraphicsDevice::TickFrameRate()
{
    static auto sTimer = Timer::StartNew();
    static auto sCounter = 0;
    
    if (sTimer.Elapsed() >= 1.0)
    {
        sTimer.Subtract(1);
    
        std::wostringstream wstr;    
        wstr << sCounter << L" FPS" << std::endl;
        // mFpsTex = mWriter.Render(wstr.str().c_str());
        sCounter = 0;
    }
    
    ++sCounter;
}

