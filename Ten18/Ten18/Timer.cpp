#include "Ten18/Timer.h"

LARGE_INTEGER Ten18::Timer::sFrequency = {};
    
void Ten18::Timer::Initialize()
{
    Expect.NotZero = QueryPerformanceFrequency(&sFrequency);
    LARGE_INTEGER dummyForReturnValueTestDoneJustThisOnce;
    Expect.NotZero = QueryPerformanceCounter(&dummyForReturnValueTestDoneJustThisOnce);
}
