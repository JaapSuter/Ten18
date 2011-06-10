#include "Ten18/PCH.h"
#include "Ten18/Graphics/GraphicsDevice.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Graphics/Display.h"
#include "Ten18/Graphics/Vertex.h"
#include "Ten18/Timer.h"
#include "Ten18/Expect.h"
#include "Ten18/Scoped.h"
#include "Ten18/OnExit.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Tracer.h"
#include "Ten18/Content/Index.h"

using namespace Ten18;
using namespace Ten18::Graphics;
using namespace Ten18::COM;

template<class T>
static void SetDebugName(const COM::COMPtr<T>& ptr, const char name[])
{
    #ifdef _DEBUG
        if (ptr)
            ptr->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name) / sizeof(*name), name);
    #else
        UNREFERENCED_PARAMETER(ptr);
        UNREFERENCED_PARAMETER(name);
    #endif
}

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
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT 
               | Ten18_IF_DEBUG_ELSE(D3D11_CREATE_DEVICE_DEBUG, 0);
    auto f11 = D3D_FEATURE_LEVEL_11_0;
    Expect.HR = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                            flags, nullptr, 0,
                            D3D11_SDK_VERSION, mD3D11Device.AsTypedDoubleStar(), &f11, mImmediateContext.AsTypedDoubleStar());
    SetDebugName(mD3D11Device, "D3D11Device");
    SetDebugName(mImmediateContext, "ImmediateContext");

    if (flags & D3D11_CREATE_DEVICE_DEBUG)
        mD3D11Device.QueryInto(mD3D11Debug);

    mD3D11Device.QueryInto(mDXGIDevice1);
    Expect.HR = mDXGIDevice1->GetParent(mDXGIAdapter1.GetUUID(), mDXGIAdapter1.AsVoidDoubleStar());
    Expect.HR = mDXGIAdapter1->GetParent(mDXGIFactory1.GetUUID(), mDXGIFactory1.AsVoidDoubleStar());

    Expect.HR = mDXGIDevice1->SetMaximumFrameLatency(0);

    const float hsw = 2048.0f;
    const float hsh = 1024.0f;
    const float hso = 0;
    const float tsw = 2048;
    const float tsh = 1024;
    Vertex vertices[] =
    {
        { XMFLOAT3(hso + 0,     0, 0.5f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2( 0, 0 ) },
        { XMFLOAT3(hso + hsw,   0, 0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2( hsw / tsw, 0 ) },
        { XMFLOAT3(hso +   0, hsh, 0.5f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2( 0, hsh / tsh ) },
        { XMFLOAT3(hso + hsw, hsh, 0.5f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2( hsw / tsw, hsh / tsh) },
    };

    InitializeShaders(L"Ten18/Content/Shaders/Capture.vso", L"Ten18/Content/Shaders/Capture.pso", mCaptureVertexShader, mCapturePixelShader);
    InitializeShaders(L"Ten18/Content/Shaders/GrayCode.vso", L"Ten18/Content/Shaders/Capture.pso", mGrayCodeVertexShader, mGrayCodePixelShader);

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = sizeof(vertices);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pSysMem = vertices;
    Expect.HR = mD3D11Device->CreateBuffer(&vbd, &subResourceData, mVertexBuffer.AsTypedDoubleStar());
    SetDebugName(mVertexBuffer, "Vertex Buffer");

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(Constants);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;
    Expect.HR = mD3D11Device->CreateBuffer(&cbd, NULL, mConstantBuffer.AsTypedDoubleStar());
    SetDebugName(mConstantBuffer, "Constant Buffer");

    const auto& entry = Content::Index::Get(L"Ten18/Content/Images/Panorama.jpg");
    Expect.HR = D3DX11CreateShaderResourceViewFromMemory(mD3D11Device.Raw(), entry.Data, entry.Size, nullptr, nullptr, mTextureRV.AsTypedDoubleStar(), nullptr);
    SetDebugName(mTextureRV, "Shader Resource View");
    
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;    
    Expect.HR = mD3D11Device->CreateSamplerState(&sampDesc, mSamplerLinear.AsTypedDoubleStar());
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
    Expect.HR = mD3D11Device->CreateBlendState(&blendDesc, mAlphaBlendState.AsTypedDoubleStar());
    SetDebugName(mAlphaBlendState, "AlphaBlend");

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = false;
    Expect.HR = mD3D11Device->CreateRasterizerState(&rasterDesc, mRasterizerState.AsTypedDoubleStar());
    SetDebugName(mRasterizerState, "Solid");

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    mImmediateContext->IASetVertexBuffers(0, 1, mVertexBuffer.AsUnsafeArrayOfOne(), &stride, &offset);
    mImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    mImmediateContext->IASetInputLayout(mVertexLayout.Raw());

    mImmediateContext->OMSetBlendState(mAlphaBlendState.Raw(), 0, UINT_MAX);
    mImmediateContext->RSSetState(mRasterizerState.Raw());

    mImmediateContext->PSSetShaderResources(0, 1, mTextureRV.AsUnsafeArrayOfOne());
    mImmediateContext->PSSetSamplers(0, 1, mSamplerLinear.AsUnsafeArrayOfOne());

    mImmediateContext->PSSetShader(mCapturePixelShader.Raw(), nullptr, 0);
    mImmediateContext->PSSetConstantBuffers(0, 1, mConstantBuffer.AsUnsafeArrayOfOne());
        
    mImmediateContext->VSSetShader(mCaptureVertexShader.Raw(), nullptr, 0);
    mImmediateContext->VSSetConstantBuffers(0, 1, mConstantBuffer.AsUnsafeArrayOfOne());

    mWriter.Initialize(*mDXGIAdapter1, *mD3D11Device);
}

void GraphicsDevice::InitializeShaders(const wchar_t* vsid, const wchar_t* psid, COM::COMPtr<ID3D11VertexShader>& vs, COM::COMPtr<ID3D11PixelShader>& ps)
{
    const auto& vsBlob = Content::Index::Get(vsid);
    Expect.HR = mD3D11Device->CreateVertexShader(vsBlob.Data, vsBlob.Size, nullptr, vs.AsTypedDoubleStar());    
    SetDebugName(vs, "Vertex Shader");
    
    const auto& psBlob = Content::Index::Get(psid);
    Expect.HR = mD3D11Device->CreatePixelShader(psBlob.Data, psBlob.Size, nullptr, ps.AsTypedDoubleStar());
    SetDebugName(ps, "Pixel Shader");

    if (!mVertexLayout)
    {
        Expect.HR = mD3D11Device->CreateInputLayout(Vertex::Layout, Vertex::LayoutCount, vsBlob.Data, vsBlob.Size, mVertexLayout.AsTypedDoubleStar());
        SetDebugName(mVertexLayout, "Vertex Layout");
    }
}

GraphicsDevice::~GraphicsDevice()
{
    if (mImmediateContext)
    {
        mImmediateContext->ClearState();
        mImmediateContext->Flush();
    }
}

void GraphicsDevice::Render()
{
    TickFrameRate();
    
    std::for_each(mSwapChains.begin(), mSwapChains.end(), [] (SwapChain* sw) { sw->Tick(); });
    
    if (mReposeInducedDwmFlushReq)
        DwmFlush();
    mReposeInducedDwmFlushReq = false;

    std::for_each(mTickSignalRequests.cbegin(), mTickSignalRequests.cend(), [] (const std::function<void ()>& func) { func(); });
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
        Expect.HR = mD3D11Device->CreateTexture2D(&td, nullptr, mDynamicTextures[i].AsTypedDoubleStar());

    for (int i = 0; i < DynamicTextureCount; ++i)
        Expect.HR = mD3D11Device->CreateShaderResourceView(mDynamicTextures[i].Raw(), nullptr, mDynamicShaderResourceViews[i].AsTypedDoubleStar());
    
    mDynamicTexturesLRU = 0;
}

void GraphicsDevice::TickDynamicTextures(Image::Ptr&& img)
{
    if (!img)
    {
        mImmediateContext->PSSetShaderResources(0, 1, mDynamicShaderResourceViews[mDynamicTexturesLRU].AsUnsafeArrayOfOne());
        return;
    }

    Ten18_ASSERT(img->BytesPerPixel() == 4);
    
    mDynamicTexturesLRU = (mDynamicTexturesLRU + 1) % DynamicTextureCount;
    auto& lru = mDynamicTextures[mDynamicTexturesLRU];
    
    // Todo, Jaap Suter, April 2011: figure out why D3D11_MAP_FLAG_DO_NOT_WAIT doesn't jive with D3D11_MAP_WRITE_DISCARD here.
    D3D11_MAPPED_SUBRESOURCE msr = {};
    {
        auto hr = mImmediateContext->Map(lru.Raw(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
        {
            DebugOut("DXGI_ERROR_WAS_STILL_DRAWING");
            return;
        }
        else Expect.HR = hr;

        OnExit oe([&] { mImmediateContext->Unmap(lru.Raw(), 0); });
    }

    if (static_cast<int>(msr.RowPitch) == img->RowPitch())
        std::memcpy(msr.pData, img->DataAs<void>(), img->Size());
    else for (int y = 0; y < img->Height(); ++y)
    {
        auto dst = static_cast<char*>(msr.pData) + y * msr.RowPitch;
        auto src = img->DataAs<char>() + y * img->RowPitch();
        auto len = img->Width() * img->BytesPerPixel();
        std::memcpy(dst, src, len);
    }

    mImmediateContext->PSSetShaderResources(0, 1, mDynamicShaderResourceViews[mDynamicTexturesLRU].AsUnsafeArrayOfOne());
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
        mFpsTex = mWriter.Render(wstr.str().c_str());
        sCounter = 0;
    }
    
    ++sCounter;
}

