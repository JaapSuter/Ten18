#include "Ten18/PCH.h"
#include "Ten18/Graphics/Writer.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Graphics/Vertex.h"
#include "Ten18/Expect.h"
#include "Ten18/Scoped.h"
#include "Ten18/OnExit.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Tracer.h"

using namespace Ten18;
using namespace Ten18::Graphics;
using namespace Ten18::COM;

Writer::Writer()
    : mDXGIAdapter2(), mD3D11Device1()
{}

Writer::~Writer()
{
    mDXGIAdapter2 = nullptr;
    mD3D11Device1 = nullptr;

    // mD3D10Device1->ClearState();
    // mD3D10Device1->Flush();    
}

void Writer::Initialize(IDXGIAdapter2& dxgiAdapter2, IDXGIDevice2& dxgiDevice2, ID3D11Device1& d3D11Device1)
{
    Ten18_ASSERT(mDXGIAdapter2 == nullptr && mD3D11Device1 == nullptr);
    
    mDXGIAdapter2 = &dxgiAdapter2;
    mDXGIDevice1 = &dxgiDevice2;
    mD3D11Device1 = &d3D11Device1;

    D2D1_FACTORY_OPTIONS opt = {};
    opt.debugLevel = Ten18_IF_DEBUG_ELSE(D2D1_DEBUG_LEVEL_WARNING, D2D1_DEBUG_LEVEL_NONE);
    COM::COMPtr<ID2D1Factory> factory;
    Ten18_EXPECT.HR = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, opt, factory.AsTypedDoubleStar());
    factory.QueryInto(mD2D1Factory1);

    Ten18_EXPECT.HR = mD2D1Factory1->CreateDevice(mDXGIDevice1, mD2D1Device.AsTypedDoubleStar());
    Ten18_EXPECT.HR = mD2D1Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, mD2D1DeviceContext.AsTypedDoubleStar());

    // const UINT flags = D3D10_CREATE_DEVICE_SINGLETHREADED
    //                  // | D3D10_CREATE_DEVICE_BGRA_SUPPORT
    //                  | Ten18_IF_DEBUG_ELSE(D3D10_CREATE_DEVICE_DEBUG, 0);
    // const auto level = D3D10_FEATURE_LEVEL_10_1;
    // Ten18_EXPECT.HR = D3D10CreateDevice1(mDXGIAdapter2, D3D10_DRIVER_TYPE_HARDWARE, nullptr,
    //                         flags, level, D3D10_1_SDK_VERSION, mD3D10Device1.AsTypedDoubleStar());
    // 
    // if (flags & D3D10_CREATE_DEVICE_DEBUG)
    // mD3D10Device1.QueryInto(mD3D10Debug);

    const auto fmt = DXGI_FORMAT_B8G8R8A8_UNORM;

    D3D11_TEXTURE2D_DESC td = {};    
    td.Format = fmt;
    td.Width = 2048;
    td.Height = 1024;    
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    // td.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE; // D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    
    Ten18_EXPECT.HR = mD3D11Device1->CreateTexture2D(&td, nullptr, mD3D11Texture2D.AsTypedDoubleStar());

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = fmt;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    Ten18_EXPECT.HR = mD3D11Device1->CreateShaderResourceView(mD3D11Texture2D.Raw(), &shaderResourceViewDesc, mD3D11ShaderResourceView.AsTypedDoubleStar());
    
    // {
    //     HANDLE directWriteHandle = nullptr;
    //     COMPtr<IDXGIResource> directWriteResource;
    //     mD3D11Texture2D.QueryInto(directWriteResource);
    //     Ten18_EXPECT.HR = directWriteResource->GetSharedHandle(&directWriteHandle);
    //     Ten18_EXPECT.HR = mD3D10Device1->OpenSharedResource(directWriteHandle, __uuidof(mD3D10Texture2D.Raw()), mD3D10Texture2D.AsVoidDoubleStar());
    // }
    
    // const float dpiX = 96.0f;
    // const float dpiY = 96.0f;
    // const auto d2drt = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(fmt, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);

    // mD3D11Texture2D.QueryInto(mKeyedMutex11);
    // mD3D10Texture2D.QueryInto(mKeyedMutex10);
    // mD3D10Texture2D.QueryInto(mDXGISurface);
    
    // Ten18_EXPECT.HR = mD2D1Factory1->CreateDxgiSurfaceRenderTarget(mDXGISurface.Raw(), d2drt, mRenderTarget.AsTypedDoubleStar());    
    // Ten18_EXPECT.HR = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(mDWriteFactory.Raw()), mDWriteFactory.AsUnknownDoubleStar());
    // 
    // const auto fontSize = 93.0f;
    // const auto locale = L"en-US";
    // Ten18_EXPECT.HR = mDWriteFactory->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
    //     fontSize, locale, mDWriteTextFormat.AsTypedDoubleStar());
    // 
    // const auto strokeProps = D2D1::StrokeStyleProperties(
    //     D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE,
    //     D2D1_LINE_JOIN_MITER_OR_BEVEL, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f);
    // Ten18_EXPECT.HR = mD2D1Factory1->CreateStrokeStyle(strokeProps, nullptr, 0, mStrokeStyle.AsTypedDoubleStar());
    // 
    // Ten18_EXPECT.HR = mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), mOutlineBrush.AsTypedDoubleStar());
    // Ten18_EXPECT.HR = mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), mFillBrush.AsTypedDoubleStar());

    // Ten18_EXPECT.HR = mKeyedMutex11->AcquireSync(0, INFINITE);    
}

ID3D11ShaderResourceView* Writer::Render(const wchar_t* text)
{    
    // mKeyedMutex11->ReleaseSync(keyId10);    
    // Ten18_EXPECT.HR = mKeyedMutex10->AcquireSync(keyId10, INFINITE);

    mRenderTarget->BeginDraw();
    
    OnExit oe([&]
    { 
        Ten18_HOPE_FOR.HR = mRenderTarget->EndDraw();
        // Ten18_HOPE_FOR.HR = mKeyedMutex10->ReleaseSync(keyId11);
        // Ten18_HOPE_FOR.HR = mKeyedMutex11->AcquireSync(keyId11, INFINITE);
    });
    
    mRenderTarget->Clear();
            
    mDWriteTextLayout.Reset();
    Ten18_EXPECT.HR = mDWriteFactory->CreateTextLayout(text, std::wcslen(text), mDWriteTextFormat.Raw(), 2048, 1024, mDWriteTextLayout.AsTypedDoubleStar());   
    Ten18_EXPECT.HR = mDWriteTextLayout->Draw(nullptr, this, 28.0f, 15.0f);

    return mD3D11ShaderResourceView.Raw();
}

IFACEMETHODIMP Writer::DrawGlyphRun(__maybenull void*, FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE, __in DWRITE_GLYPH_RUN const* glyphRun, __in DWRITE_GLYPH_RUN_DESCRIPTION const*,
    IUnknown* clientDrawingEffect)
{
    Ten18_UNUSED(clientDrawingEffect);
    COM::COMPtr<ID2D1PathGeometry> path;
    Ten18_EXPECT.HR = mD2D1Factory1->CreatePathGeometry(path.AsTypedDoubleStar());        

    COM::COMPtr<ID2D1GeometrySink> sink;
    Ten18_EXPECT.HR = path->Open(sink.AsTypedDoubleStar());

    Ten18_EXPECT.HR = glyphRun->fontFace->GetGlyphRunOutline(
                glyphRun->fontEmSize,
                glyphRun->glyphIndices,
                glyphRun->glyphAdvances,
                glyphRun->glyphOffsets,
                glyphRun->glyphCount,
                glyphRun->isSideways,
                glyphRun->bidiLevel, sink.Raw());

    Ten18_EXPECT.HR = sink->Close();

    const auto matrix = D2D1::Matrix3x2F(1.0f, 0.0f, 0.0f, 1.0f, baselineOriginX, baselineOriginY);

    COM::COMPtr<ID2D1TransformedGeometry> geometry;
    Ten18_EXPECT.HR = mD2D1Factory1->CreateTransformedGeometry(path.Raw(), &matrix, geometry.AsTypedDoubleStar());

    const auto strokeWidth = 13.7f;
    mRenderTarget->DrawGeometry(geometry.Raw(), mOutlineBrush.Raw(), strokeWidth, mStrokeStyle.Raw());
    mRenderTarget->FillGeometry(geometry.Raw(), mFillBrush.Raw());

    return S_OK;
}

IFACEMETHODIMP Writer::DrawUnderline(__maybenull void*, FLOAT,
    FLOAT, __in DWRITE_UNDERLINE const*, IUnknown*)
{
    return S_OK;
}

IFACEMETHODIMP Writer::DrawStrikethrough(__maybenull void*, FLOAT,
    FLOAT, __in DWRITE_STRIKETHROUGH const*, IUnknown*)
{
    return S_OK;
}

IFACEMETHODIMP Writer::DrawInlineObject(__maybenull void*, FLOAT, FLOAT,
    IDWriteInlineObject*, BOOL, BOOL, IUnknown*)
{
    return E_NOTIMPL;
}

IFACEMETHODIMP Writer::IsPixelSnappingDisabled(__maybenull void*, __out BOOL* isDisabled)
{
    *isDisabled = FALSE;
    return S_OK;
}

IFACEMETHODIMP Writer::GetCurrentTransform(__maybenull void*, __out DWRITE_MATRIX* transform)
{
    mRenderTarget->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
    return S_OK;
}

IFACEMETHODIMP Writer::GetPixelsPerDip(__maybenull void*, __out FLOAT* pixelsPerDip)
{
    *pixelsPerDip = 1.0f;
    return S_OK;
}
