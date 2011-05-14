#pragma once
#include "Ten18/Timer.h"
#include "Ten18/Util.h"

namespace Ten18
{
    #define Ten18_TRACER() ::Ten18::Tracer tracer(__FILE__ ## "(" ## Ten18_STRINGIFY(__LINE__) ## "): " ## __FUNCTION__)

    class Tracer
    {
    public:

        Tracer(const char msg[])
            : mMsg(msg), mTimer(Timer::StartImmediately), mStopped()
        {
            ::Ten18::DebugOut("%*s%s begin", sIndent * sNumSpacesPerIndent, "", mMsg);
            ++sIndent;
        }

        ~Tracer()
        {
            Stop();            
        }
        
    private:

        Tracer(const Tracer&);
        Tracer& operator = (const Tracer&);

        void Stop()
        {
            if (mStopped)
                return;
            
            mTimer.Stop();
            mStopped = true;            
            --sIndent;

            ::Ten18::DebugOut("%*s%s end: %1.2f ms", sIndent * sNumSpacesPerIndent, "", mMsg, mTimer.Elapsed() * 1000);
        }
        
        const char* mMsg;
        
        Timer mTimer;
        bool mStopped;

        static const auto sNumSpacesPerIndent = 4;
        static int sIndent;
    };
}
