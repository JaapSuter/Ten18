#include "Ten18/PCH.h"
#include "Ten18/Graphics/Writer.h"
#include "Ten18/Graphics/SwapChain.h"
#include "Ten18/Graphics/Vertex.h"
#include "Ten18/Expect.h"
#include "Ten18/Scoped.h"
#include "Ten18/OnExit.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/EmbeddedResourceStream.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Tracer.h"

using namespace Ten18;
using namespace Ten18::Graphics;
using namespace Ten18::COM;

Writer::Writer()
    : mDXGIAdapter1(), mD3D11Device()
{}

Writer::~Writer()
{
    mDXGIAdapter1 = nullptr;
    mD3D11Device = nullptr;

    mD3D10Device1->ClearState();
    mD3D10Device1->Flush();    
}

void Writer::Initialize(IDXGIAdapter1& dxgiAdapter1, ID3D11Device& d3D11Device)
{
    Ten18_ASSERT(mDXGIAdapter1 == nullptr && mD3D11Device == nullptr);
    
    mDXGIAdapter1 = &dxgiAdapter1;
    mD3D11Device = &d3D11Device;

    const UINT flags = D3D10_CREATE_DEVICE_SINGLETHREADED
                     | D3D10_CREATE_DEVICE_BGRA_SUPPORT
                     | Ten18_IF_DEBUG_ELSE(D3D10_CREATE_DEVICE_DEBUG, 0);
    const auto level = D3D10_FEATURE_LEVEL_10_1;
    Expect.HR = D3D10CreateDevice1(mDXGIAdapter1, D3D10_DRIVER_TYPE_HARDWARE, nullptr,
                            flags, level, D3D10_1_SDK_VERSION, mD3D10Device1.AsTypedDoubleStar());

    if (flags & D3D10_CREATE_DEVICE_DEBUG)
        mD3D10Device1.QueryInto(mD3D10Debug);

    const auto fmt = DXGI_FORMAT_B8G8R8A8_UNORM;

    D3D11_TEXTURE2D_DESC td = {};    
    td.Format = fmt;
    td.Width = 2048;
    td.Height = 1024;    
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.SampleDesc.Count = 1;
    td.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    Expect.HR = mD3D11Device->CreateTexture2D(&td, nullptr, mD3D11Texture2D.AsTypedDoubleStar());

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = fmt;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    Expect.HR = mD3D11Device->CreateShaderResourceView(mD3D11Texture2D.Raw(), &shaderResourceViewDesc, mD3D11ShaderResourceView.AsTypedDoubleStar());
    
    {
        HANDLE directWriteHandle = nullptr;
        COMPtr<IDXGIResource> directWriteResource;
        mD3D11Texture2D.QueryInto(directWriteResource);
        Expect.HR = directWriteResource->GetSharedHandle(&directWriteHandle);
        Expect.HR = mD3D10Device1->OpenSharedResource(directWriteHandle, __uuidof(mD3D10Texture2D.Raw()), mD3D10Texture2D.AsVoidDoubleStar());
    }
    
    D2D1_FACTORY_OPTIONS opt = {};
    opt.debugLevel = Ten18_IF_DEBUG_ELSE(D2D1_DEBUG_LEVEL_WARNING, D2D1_DEBUG_LEVEL_NONE);
    Expect.HR = D2D1CreateFactory<ID2D1Factory>(D2D1_FACTORY_TYPE_SINGLE_THREADED, opt, mD2D1Factory.AsTypedDoubleStar());
    
    const float dpiX = 96.0f;
    const float dpiY = 96.0f;
    const auto d2drt = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(fmt, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);

    mD3D11Texture2D.QueryInto(mKeyedMutex11);
    mD3D10Texture2D.QueryInto(mKeyedMutex10);
    mD3D10Texture2D.QueryInto(mDXGISurface);
    
    Expect.HR = mD2D1Factory->CreateDxgiSurfaceRenderTarget(mDXGISurface.Raw(), d2drt, mRenderTarget.AsTypedDoubleStar());    
    Expect.HR = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(mDWriteFactory.Raw()), mDWriteFactory.AsUnknownDoubleStar());

    const auto fontSize = 93.0f;
    const auto locale = L"en-US";
    Expect.HR = mDWriteFactory->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        fontSize, locale, mDWriteTextFormat.AsTypedDoubleStar());

    const auto strokeProps = D2D1::StrokeStyleProperties(
        D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE, D2D1_CAP_STYLE_SQUARE,
        D2D1_LINE_JOIN_MITER_OR_BEVEL, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f);
    Expect.HR = mD2D1Factory->CreateStrokeStyle(strokeProps, nullptr, 0, mStrokeStyle.AsTypedDoubleStar());

    Expect.HR = mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), mOutlineBrush.AsTypedDoubleStar());
    Expect.HR = mRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), mFillBrush.AsTypedDoubleStar());

    Expect.HR = mKeyedMutex11->AcquireSync(0, INFINITE);    
}

ID3D11ShaderResourceView* Writer::Render(const wchar_t* text)
{    
    mKeyedMutex11->ReleaseSync(keyId10);
    
    Expect.HR = mKeyedMutex10->AcquireSync(keyId10, INFINITE);    
    mRenderTarget->BeginDraw();
    
    OnExit oe([&]
    { 
        HopeFor.HR = mRenderTarget->EndDraw();
        HopeFor.HR = mKeyedMutex10->ReleaseSync(keyId11);
        HopeFor.HR = mKeyedMutex11->AcquireSync(keyId11, INFINITE);
    });
    
    mRenderTarget->Clear();
            
    mDWriteTextLayout.Reset();
    Expect.HR = mDWriteFactory->CreateTextLayout(text, std::wcslen(text), mDWriteTextFormat.Raw(), 2048, 1024, mDWriteTextLayout.AsTypedDoubleStar());   
    Expect.HR = mDWriteTextLayout->Draw(nullptr, this, 28.0f, 15.0f);

    return mD3D11ShaderResourceView.Raw();
}

IFACEMETHODIMP Writer::DrawGlyphRun(__maybenull void*, FLOAT baselineOriginX, FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE, __in DWRITE_GLYPH_RUN const* glyphRun, __in DWRITE_GLYPH_RUN_DESCRIPTION const*,
    IUnknown* clientDrawingEffect)
{
    UNREFERENCED_PARAMETER(clientDrawingEffect);
    COM::COMPtr<ID2D1PathGeometry> path;
    Expect.HR = mD2D1Factory->CreatePathGeometry(path.AsTypedDoubleStar());        

    COM::COMPtr<ID2D1GeometrySink> sink;
    Expect.HR = path->Open(sink.AsTypedDoubleStar());

    Expect.HR = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel, sink.Raw());

    Expect.HR = sink->Close();

    const auto matrix = D2D1::Matrix3x2F(1.0f, 0.0f, 0.0f, 1.0f, baselineOriginX, baselineOriginY);

    COM::COMPtr<ID2D1TransformedGeometry> geometry;
    Expect.HR = mD2D1Factory->CreateTransformedGeometry(path.Raw(), &matrix, geometry.AsTypedDoubleStar());

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
