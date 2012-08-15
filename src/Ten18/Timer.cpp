#include "Ten18/Timer.h"

LARGE_INTEGER Ten18::Timer::sFrequency = {};
    
void Ten18::Timer::Initialize()
{
    Ten18_EXPECT.NotZero = QueryPerformanceFrequency(&sFrequency);
    LARGE_INTEGER dummyForReturnValueTestDoneJustThisOnce;
    Ten18_EXPECT.NotZero = QueryPerformanceCounter(&dummyForReturnValueTestDoneJustThisOnce);
}
