#pragma once
#include "Ten18/Graphics/Image.h"

namespace Ten18 { namespace Capture {

    class CaptureSource
    {
    public:

        typedef Graphics::Image Image;

        const dx::XMFLOAT2A& Size() const { return mSize; }
        int BytesPerPixel() const { return mBytesPerPixel; }
        
        virtual ~CaptureSource() = 0;
        Graphics::Image::Ptr Latest();

        virtual void Tick() = 0;

        static void* New();

        void get_Size(dx::XMFLOAT2& ret);
        
    protected:

        dx::XMFLOAT2A mSize;
        int mBytesPerPixel;

        Graphics::Image::Ptr mLatest;

        CaptureSource();

    private:
        CaptureSource(const CaptureSource&);
        CaptureSource& operator = (const CaptureSource&);
    };

}}
