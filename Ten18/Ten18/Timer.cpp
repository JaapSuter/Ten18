#include "Ten18/Timer.h"

LARGE_INTEGER Ten18::Timer::sFrequency = {};
    
void Ten18::Timer::Initialize()
{
    Expect.NotZero = QueryPerformanceFrequency(&sFrequency);
    LARGE_INTEGER dummyForReturnValueTestDoneJustThisOnce;
    Expect.NotZero = QueryPerformanceCounter(&dummyForReturnValueTestDoneJustThisOnce);

    TIMECAPS tc = {};
    Expect.EqualTo<MMRESULT>(MMSYSERR_NOERROR) = timeGetDevCaps(&tc, sizeof(tc));
    DebugOut("timeGetDevCaps min: %d ms, max: %d ms", tc.wPeriodMin, tc.wPeriodMax);
}
