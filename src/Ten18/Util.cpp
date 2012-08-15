#include "Ten18/Util.h"
#include "Ten18/Scoped.h"
#include "Ten18/Format.h"
#include "Ten18/Expect.h"

std::vector<HMODULE> Ten18::Util::EnumerateModules()
{
    std::vector<HMODULE> modules;

    auto snapshot = MakeScoped(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0), [] (HANDLE snapshot) { Ten18_HOPE_FOR.True = CloseHandle(snapshot); });
    Ten18_EXPECT.Not(INVALID_HANDLE_VALUE) = static_cast<HANDLE>(snapshot);
    
    MODULEENTRY32 me = {};
    me.dwSize = sizeof(me);
    const auto minSizeOfInterest = FIELD_OFFSET(MODULEENTRY32, hModule) + sizeof(me.hModule);
    if (Module32First(snapshot, &me))
    {
        do
        {
            if (me.dwSize >= minSizeOfInterest)
                modules.push_back(me.hModule);

            Format("Module: %0", me.szModule).DebugOut();
                
            me.dwSize = sizeof(me);
        }
        while (Module32Next(snapshot, &me));
    }
    
    return modules;
}

std::wstring Ten18::to_string(const GUID& guid)
{
    const int len = 39;
    std::wstring ret(len, ' ');
    Expect.EqualTo(len) = StringFromGUID2(guid, &ret[0], len + 1);
    return ret;
}

bool Ten18::Util::FileExists(const wchar_t *fileName)
{
    return INVALID_FILE_ATTRIBUTES != GetFileAttributes(fileName);
}

std::string Ten18::Util::Narrow(const std::wstring& wstr)
{
    return std::string(wstr.begin(), wstr.end());
}

int Ten18::Util::EnumerateNativeThreads(bool traceFullInfo, const char* msg)
{
    Ten18_UNUSED(traceFullInfo);
    DebugOut(msg);

    int numThreads = 0;
    auto snapshot = MakeScoped(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0), [] (HANDLE snapshot) { Ten18_HOPE_FOR.True = CloseHandle(snapshot); });
    Ten18_EXPECT.Not(INVALID_HANDLE_VALUE) = static_cast<HANDLE>(snapshot);
    
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
                    auto thread = MakeScoped(OpenThread(THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID), [] (HANDLE t) { Ten18_HOPE_FOR.NotZero = CloseHandle(t); });
                    Ten18_EXPECT.NotNull = static_cast<HANDLE>(thread);

                    ULONG64 cycleTime = 0;
                    Ten18_EXPECT.True = QueryThreadCycleTime(thread, &cycleTime);
                    
                    FILETIME create, exit, kernel, user;
                    Ten18_EXPECT.True = GetThreadTimes(thread, &create, &exit, &kernel, &user);

                    Format("\tNative Thread %0 - Create: %1, Exit: %2, Kernel: %3, User: %4, Cycles: %5",
                        te.th32ThreadID, create.dwLowDateTime, exit.dwLowDateTime, kernel.dwLowDateTime, user.dwLowDateTime, static_cast<int>(cycleTime)).DebugOut();
                }
                
            te.dwSize = sizeof(te);
        }
        while (Thread32Next(snapshot, &te));
    }
    
    Format("Number of Native Threads: %0", numThreads).DebugOut();

    return numThreads;
}

