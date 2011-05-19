#include "Ten18/Capture/CLEyeCapture.h"
#include "Ten18/Expect.h"
#include "Ten18/Timer.h"
#include "Ten18/Image.h"
#include "../Libraries/CLEyeMulticam.h"

using namespace Ten18;
using namespace Ten18::Capture;

int CLEyeCapture::Count()
{
    return 1; // CLEyeGetCameraCount();
}

CaptureSource::Ptr CLEyeCapture::Get(int idx)
{
    return MakeUniquePtr<CLEyeCapture>(CLEyeGetCameraUUID(idx));
}

CLEyeCapture::CLEyeCapture(const GUID& guid)
    : mGuid(guid), mCam()
{   
    /*
    const auto fps = 43.0f;
    const auto resolution = CLEYE_VGA;
    const auto mode = CLEYE_COLOR_PROCESSED;
    
    Expect.True = mCam = CLEyeCreateCamera(guid, mode, resolution, fps);
    Expect.True = CLEyeCameraGetFrameDimensions(mCam, mWidth, mHeight);
    
    mBytesPerPixel = mode == CLEYE_COLOR_PROCESSED ? 4 : 1;
    
    Expect.True = CLEyeCameraStart(mCam);
    */
    mWidth = mHeight = 256;
    mBytesPerPixel = 4;
}

CLEyeCapture::~CLEyeCapture()
{
    // HopeFor.NotZero = CLEyeCameraStop(mCam);
    // HopeFor.NotZero = CLEyeDestroyCamera(mCam);
    // mCam = nullptr;
}

void CLEyeCapture::Tick()
{
    auto img = Image::New(mWidth, mHeight, mBytesPerPixel);
    
    auto gotNewFrame = true; // CLEyeCameraGetFrame(mCam, img->DataAs<unsigned char>(), 0);
    {
        Sleep(2);
    }



    if (gotNewFrame)
        mLatest = std::move(img);
    else
        DebugOut("No Frame");
}
