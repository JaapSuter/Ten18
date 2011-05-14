#include "Ten18/Cli.h"
#pragma unmanaged
#include "Ten18/Interop/Interop.h"
extern "C" __declspec(dllimport) void __stdcall NativeRegisterForTickSignal(void* func);
#pragma managed

using namespace System;
using namespace System::Threading::Tasks;
using namespace System::Collections::Concurrent;
using namespace System::Runtime::InteropServices;
using namespace Ten18;

void Cli::ProcessTickSignals()
{
    while (!sWaitingForSignal->IsEmpty)
    {
        TaskSignal^ ts;
        if (sWaitingForSignal->TryDequeue(ts))
        {
            ts->SetResult(0);
        }
    }
}

void Cli::Initialize()
{
    sWaitingForSignal = gcnew TaskQueue();
    sDelegateTickSignals = gcnew DelegateTickSignals(&Cli::ProcessTickSignals);

    typedef void (__stdcall *FunPtr)();
    IntPtr ip = Marshal::GetFunctionPointerForDelegate(sDelegateTickSignals);
    NativeRegisterForTickSignal(ip.ToPointer());
}

Task^ Cli::TickAsync()
{
    auto tcs = gcnew TaskCompletionSource<int>();
    sWaitingForSignal->Enqueue(tcs);    
    return tcs->Task;
}

