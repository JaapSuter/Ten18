#include "Ten18/Capture/CaptureSource.h"

using namespace Ten18;
using namespace Ten18::Capture;

CaptureSource::CaptureSource()
    : mBytesPerPixel(), mWidth(), mHeight()
{
}

CaptureSource::~CaptureSource()
{
}

Image::Ptr CaptureSource::Latest()
{
    return std::move(mLatest);
}
