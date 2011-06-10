#pragma once
#include "Ten18/Graphics/Image.h"

namespace Ten18 { namespace Capture {

    class CaptureSource
    {
    public:

        typedef Graphics::Image Image;

        const XMFLOAT2A& Size() const { return mSize; }
        int BytesPerPixel() const { return mBytesPerPixel; }
        
        virtual ~CaptureSource() = 0;
        Graphics::Image::Ptr Latest();

        virtual void Tick() = 0;
        
        #include "Ten18/Capture/CaptureSource.Generated.h"

    protected:

        XMFLOAT2A mSize;
        int mBytesPerPixel;

        Graphics::Image::Ptr mLatest;

        CaptureSource();

    private:
        CaptureSource(const CaptureSource&);
        CaptureSource& operator = (const CaptureSource&);
    };

}}
