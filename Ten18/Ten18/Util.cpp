#include "Ten18/Util.h"
#include "Ten18/Scoped.h"
#include "Ten18/Expect.h"

std::vector<HMODULE> Ten18::Util::EnumerateModules()
{
    std::vector<HMODULE> modules;

    auto snapshot = MakeScoped(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0), [] (HANDLE snapshot) { HopeFor.True = CloseHandle(snapshot); });
    Expect.NotEqualTo(INVALID_HANDLE_VALUE) = static_cast<HANDLE>(snapshot);
    
    MODULEENTRY32 me = {};
    me.dwSize = sizeof(me);
    const auto minSizeOfInterest = FIELD_OFFSET(MODULEENTRY32, hModule) + sizeof(me.hModule);
    if (Module32First(snapshot, &me))
    {
        do
        {
            if (me.dwSize >= minSizeOfInterest)
                modules.push_back(me.hModule);

            DebugOut("Module: %S", me.szModule);
                
            me.dwSize = sizeof(me);
        }
        while (Module32Next(snapshot, &me));
    }
    
    return modules;
}

int Ten18::Util::EnumerateNativeThreads(bool traceFullInfo, const char* msg)
{
    UNREFERENCED_PARAMETER(traceFullInfo);
    DebugOut("%s", msg);

    int numThreads = 0;
    auto snapshot = MakeScoped(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0), [] (HANDLE snapshot) { HopeFor.True = CloseHandle(snapshot); });
    Expect.NotEqualTo(INVALID_HANDLE_VALUE) = static_cast<HANDLE>(snapshot);
    
    THREADENTRY32 te = {};
    te.dwSize = sizeof(te);
    const auto minSizeOfInterest = (std::min)((FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)),
                                              (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32ThreadID)));
    if (Thread32First(snapshot, &te))
    {
        do
        {
            if (te.dwSize >= minSizeOfInterest)
                if (te.th32OwnerProcessID == GetCurrentProcessId())
                {                
                    ++numThreads;
                    auto thread = MakeScoped(OpenThread(THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID), [] (HANDLE t) { HopeFor.NotZero = CloseHandle(t); });
                    Expect.NotNull = static_cast<HANDLE>(thread);

                    ULONG64 cycleTime = 0;
                    Expect.True = QueryThreadCycleTime(thread, &cycleTime);
                    
                    FILETIME create, exit, kernel, user;
                    Expect.True = GetThreadTimes(thread, &create, &exit, &kernel, &user);

                    DebugOut("\tNative Thread %d - Create: %d, Exit: %d, Kernel: %d, User: %d, Cycles: %d\n",
                        te.th32ThreadID, create.dwLowDateTime, exit.dwLowDateTime, kernel.dwLowDateTime, user.dwLowDateTime, static_cast<int>(cycleTime));
                }
                
            te.dwSize = sizeof(te);
        }
        while (Thread32Next(snapshot, &te));
    }
    
    DebugOut("Number of Native Threads: %d\n", numThreads);

    return numThreads;
}

