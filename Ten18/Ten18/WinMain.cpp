#include "Ten18/PCH.h"
#include "Ten18/Expect.h"
#include "Ten18/OnExit.h"
#include "Ten18/Program.h"
#include "Ten18/Timer.h"
#include "Ten18/Tracer.h"
#include "Ten18/Interop/Host.h"

using namespace Ten18;
using namespace Ten18::Interop;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    Expect.True = HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    Expect.True = SetDllDirectory(L"");
    Expect.True = XMVerifyCPUSupport();
    Expect.HR = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
    
    Timer::Initialize();
    
    Host host;    
    
    return Program::Run(host);

    // Todo, Jaap Suter, May 2011, we don't call CoUninitialize because the CLR host is running past main, and while
    // multiple matching CoInit and CoUninit calls should balance out, I'm just gonnna throw this cargo cult 
    // voodoo out the window and hope for the best...
}

