#pragma once

#include "Ten18/Assert.h"

namespace Ten18
{
    class Image
    {
    public:
        typedef std::unique_ptr<Image> Ptr;

        static Ptr New(int width, int height, int bytesPerPixel);

        int Width() const { return mWidth; }
        int Height() const { return mHeight; }
        int BytesPerPixel() const { return mBytesPerPixel; }
        int RowPitch() const { return mWidth * mBytesPerPixel; }
        int Size() const { return mHeight * RowPitch(); }

        Image(int width, int height, int bytesPerPixel);
        ~Image();

        template<class T>       T* DataAs()       { return static_cast<T*>(mData); }        
        template<class T> const T* DataAs() const { return static_cast<T*>(mData); }

        const void* Data() const { return mData; }
              void* Data()       { return mData; }

        template<class Pixel>
        void ForEach(const std::function<Pixel (Pixel)>& func)
        {
            Ten18_ASSERT(sizeof(Pixel) == mBytesPerPixel);
            Pixel* p = DataAs<Pixel>();
            for (int n = mWidth * mHeight; n > 0; --n, ++p)
                *p = func(*p);
        }

    private:

        Image(const Image&);
        Image& operator = (const Image&);

        int mWidth;
        int mHeight;
        int mBytesPerPixel;
        void* mData;
    };
}
