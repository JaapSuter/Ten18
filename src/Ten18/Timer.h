#pragma once
#include "Ten18/Expect.h"

namespace Ten18
{
    class Timer
    {
    public:

        static void Initialize();

        enum CtorBehaviour
        {
            StartImmediately,
            DoNotStartQuiteYet,
        };

        explicit Timer(CtorBehaviour cb)
        {
            Ten18_ASSERT(sFrequency.QuadPart != 0);            
            if (cb == StartImmediately)
                Start();
        }

        static Timer StartNew() { return Timer(StartImmediately); }

        void Subtract(int seconds)
        {
            mBegin.QuadPart += sFrequency.QuadPart * seconds;
        }

        void Start() { Ten18_EXPECT.True = QueryPerformanceCounter(&mBegin); }        
        void Stop() { Ten18_EXPECT.True = QueryPerformanceCounter(&mEnd); }

        double Elapsed() const
        {
            Ten18_EXPECT.True = QueryPerformanceCounter(&mEnd);
            auto delta = static_cast<double>(mEnd.QuadPart - mBegin.QuadPart);
            auto ticksPerSecond = static_cast<double>(sFrequency.QuadPart);
            return delta / ticksPerSecond;
        }

    private:

        static LARGE_INTEGER sFrequency;
        LARGE_INTEGER mBegin;
        mutable LARGE_INTEGER mEnd;
    };
}
