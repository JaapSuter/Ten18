#include "Ten18/Capture/CaptureSource.h"
#include "Ten18/Capture/CLEyeCapture.h"
#include "Ten18/Capture/MediaFoundationCapture.h"

namespace Ten18 { namespace Capture {

CaptureSource::CaptureSource()
    : mSize(), mBytesPerPixel()
{
}

CaptureSource::~CaptureSource()
{
}

Graphics::Image::Ptr CaptureSource::Latest()
{
    return std::move(mLatest);
}

void* CaptureSource::New()
{
    if (MediaFoundationCapture::Count() > 0)
        return MediaFoundationCapture::Get(0);
    else if (CLEyeCapture::Count() > 0)
        return CLEyeCapture::Get(0);
    
    Ten18_FAIL_FMT("No capture sources available...");
    return nullptr;
}

void CaptureSource::get_Size(dx::XMFLOAT2& ret)
{
    ret = mSize;
}

}}
