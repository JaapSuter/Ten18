#pragma once
#include "Ten18/Image.h"

namespace Ten18 { namespace Capture {

    class CaptureSource
    {
    public:
        typedef std::unique_ptr<CaptureSource> Ptr;

        int Width() const { return mWidth; }
        int Height() const { return mHeight; }
        int BytesPerPixel() const { return mBytesPerPixel; }
        
        virtual ~CaptureSource() = 0;
        Image::Ptr Latest();

        virtual void Tick() = 0;

    protected:
        int mWidth;
        int mHeight;
        int mBytesPerPixel;

        Image::Ptr mLatest;

        CaptureSource();

    private:
        CaptureSource(const CaptureSource&);
        CaptureSource& operator = (const CaptureSource&);
    };

}}
