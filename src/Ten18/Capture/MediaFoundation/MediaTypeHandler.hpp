#ifndef PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_TYPE_HANDLER_HPP
#define PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_TYPE_HANDLER_HPP

#include "Pentacorn/Graphics/Image.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

typedef void (*IMAGE_TRANSFORM_FN)(
    unsigned char* pDest,
    LONG lDestStride,
    const unsigned char* pSrc,
    LONG lSrcStride,
    DWORD dwWidthInPixels,
    DWORD dwHeightInPixels
    );

class MediaTypeHandler
{
public:
    UINT                    m_width;
    UINT                    m_height;
    unsigned char*          m_buffer;
    Graphics::Image         image;

private:

    D3DFORMAT               m_format;
    LONG                    m_lDefaultStride;
    MFRatio                 m_PixelAR;
    MFVideoInterlaceMode    m_interlace;
    RECT                    m_rcDest;
    IMAGE_TRANSFORM_FN      m_convertFn;

private:
    
    HRESULT SetConversionFunction(REFGUID subtype);
    
public:

    MediaTypeHandler();
    ~MediaTypeHandler();
    
    void SetVideoType(IMFMediaType *pType);
    void DrawFrame(IMFMediaBuffer *pBuffer);

    bool IsFormatSupported(REFGUID subtype) const;
    const GUID* GetFormat(int index) const;

private:
    MediaTypeHandler(const MediaTypeHandler&);
    MediaTypeHandler& operator = (const MediaTypeHandler&);
};

}}}

#endif
