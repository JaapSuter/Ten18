#pragma once
#include "Ten18/Capture/CaptureSource.h"

namespace Ten18 { namespace Capture {

    class CLEyeCapture : public CaptureSource
    {
    public:
        static int Count();
        static CLEyeCapture* Get(int idx);

        explicit CLEyeCapture(const GUID& guid);
        virtual ~CLEyeCapture();
        
    private:

        virtual void Tick();
        
        CLEyeCapture(const CLEyeCapture&);
        CLEyeCapture& operator = (const CLEyeCapture&);

        GUID mGuid;
        void* mCam;
    };

}}
