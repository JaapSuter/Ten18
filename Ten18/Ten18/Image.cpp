#include "Ten18/Image.h"
#include "Ten18/Expect.h"

using namespace Ten18;

#pragma warning(disable: 4505) // Todo, Jaap Suter, April 2011
static int BytesPerPixelFor(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 16;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
            return 8;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return 4;

        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return 2;

        default:
            Ten18_ASSERT(false);
    }
}

Image::Ptr Image::New(int width, int height, int bytesPerPixel)
{
    return MakeUniquePtr<Image>(width, height, bytesPerPixel);
}

Image::Image(int width, int height, int bytesPerPixel)
    : mWidth(width), mHeight(height), mBytesPerPixel(bytesPerPixel)
{
    const auto size = width * height * bytesPerPixel;
    mData = _aligned_malloc(size, XM_CACHE_LINE_SIZE);
}

Image::~Image()
{
    _aligned_free(mData);
    mData = nullptr;
    mWidth = mHeight = mBytesPerPixel = 0;
}

