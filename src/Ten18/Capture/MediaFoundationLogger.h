#pragma once

namespace Ten18 { namespace Capture {

    class MediaFoundationLogger
    {
    public:
		static void Log(IMFMediaType& mediaType);
    private:
        MediaFoundationLogger();
    };

}}
