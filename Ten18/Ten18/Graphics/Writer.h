#pragma once

#include "Ten18/Graphics/Constants.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/Graphics/Image.h"

namespace Ten18 { namespace Graphics {

    class Writer : public IDWriteTextRenderer
    {
    public:

        Writer();
        ~Writer();

        void Initialize(IDXGIAdapter1& dxgiAdapter1, ID3D11Device& d3D11Device);
        
        ID3D11ShaderResourceView* Render(const wchar_t* text);
        
        IFACEMETHOD(IsPixelSnappingDisabled)(
            __maybenull void* clientDrawingContext,
            __out BOOL* isDisabled
            );

        IFACEMETHOD(GetCurrentTransform)(
            __maybenull void* clientDrawingContext,
            __out DWRITE_MATRIX* transform
            );

        IFACEMETHOD(GetPixelsPerDip)(
            __maybenull void* clientDrawingContext,
            __out FLOAT* pixelsPerDip
            );

        IFACEMETHOD(DrawGlyphRun)(
            __maybenull void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            __in DWRITE_GLYPH_RUN const* glyphRun,
            __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawUnderline)(
            __maybenull void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            __in DWRITE_UNDERLINE const* underline,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawStrikethrough)(
            __maybenull void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            __in DWRITE_STRIKETHROUGH const* strikethrough,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawInlineObject)(
            __maybenull void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
            );

        Ten18_QUERY_INTERFACE_BEGIN(IDWriteTextRenderer)
            Ten18_QUERY_INTERFACE_IMPL(IDWriteTextRenderer)
            Ten18_QUERY_INTERFACE_IMPL(IDWritePixelSnapping)            
        Ten18_QUERY_INTERFACE_END()

    private:
        
        Writer(const Writer&);
        Writer& operator = (const Writer&);

        IDXGIAdapter1*  mDXGIAdapter1;
        ID3D11Device*   mD3D11Device;
        
        COM::COMPtr<ID3D10Device1>              mD3D10Device1;
        COM::COMPtr<ID3D10Debug>                mD3D10Debug;
        COM::COMPtr<ID2D1Factory>               mD2D1Factory;
        COM::COMPtr<IDWriteFactory>             mDWriteFactory;
        
        COM::COMPtr<IDXGISurface>               mDXGISurface;
        COM::COMPtr<IDXGIKeyedMutex>            mKeyedMutex11;
        COM::COMPtr<IDXGIKeyedMutex>            mKeyedMutex10;
        COM::COMPtr<ID3D11Texture2D>            mD3D11Texture2D;
        COM::COMPtr<ID3D10Texture2D>            mD3D10Texture2D;
        COM::COMPtr<ID2D1RenderTarget>          mRenderTarget;
        COM::COMPtr<ID3D11ShaderResourceView>   mD3D11ShaderResourceView;

        COM::COMPtr<IDWriteTextFormat>          mDWriteTextFormat;
        COM::COMPtr<IDWriteTextLayout>          mDWriteTextLayout;
        COM::COMPtr<ID2D1SolidColorBrush>       mOutlineBrush;
        COM::COMPtr<ID2D1SolidColorBrush>       mFillBrush;
        COM::COMPtr<ID2D1StrokeStyle>           mStrokeStyle;

        const static UINT64 keyId10 = 101;
        const static UINT64 keyId11 = 11;    
    };

}}
