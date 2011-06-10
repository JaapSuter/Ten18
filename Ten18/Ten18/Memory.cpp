#pragma once

#include "Ten18/Memory.h"
#include "Ten18/Expect.h"

namespace Ten18 {

    static volatile long sAllocationCount = 0;
    static _CrtMemState sCrtMemStateAtFirstAllocation = {};

    static void LookForMemoryLeaks()
    {
        #ifdef _DEBUG
            // Despite the apparent sanity of atomic counters used below, we're not actually using sCrtMemStateAtFirstAllocation in
            // a thread-safe manner. A 'zeroeth' allocation can occur between this free and this piece of code. Alas, 
            // time marches on, and I got bigger fish to fry. One day I'm sure Jaap will get to fix that...   :-)
            _CrtMemState current = {};
            _CrtMemCheckpoint(&current);

            _CrtMemState difference = {};
            if (_CrtMemDifference(&difference, &sCrtMemStateAtFirstAllocation, &current))
            {
                const size_t expectedClrHostingNormalBlockLeakCount = 2;
                if (difference.lCounts[_NORMAL_BLOCK] > expectedClrHostingNormalBlockLeakCount || difference.lCounts[_CLIENT_BLOCK] > 0)
                {            
                    _CrtMemDumpStatistics(&difference);
                    Expect.Zero = _CrtDumpMemoryLeaks();
                }
            }
        #endif
    }

void Memory::Initialize()
{
    Expect.NotEqualTo(-1) = _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    Expect.NotEqualTo(-1) = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    Expect.NotEqualTo(-1) = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);

    Expect.NotEqualTo(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    Expect.NotEqualTo(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    Expect.NotEqualTo(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    const auto crtDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    const auto addDbgFlags = _CRTDBG_CHECK_EVERY_128_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF;
    Expect.EqualTo(crtDbgFlags) = _CrtSetDbgFlag(crtDbgFlags | addDbgFlags);
    (void)addDbgFlags;
}

void Memory::Shutdown()
{
    Ten18_ASSERT_MSG(0 == sAllocationCount, "Memory leaks: %d allocations have not been freed.", sAllocationCount);
    
    LookForMemoryLeaks();
        
}

void* Memory::Alloc(std::size_t size, const char* file, int line)
{
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);

    const long first = 1;
    if (first == InterlockedIncrement(&sAllocationCount))
        _CrtMemCheckpoint(&sCrtMemStateAtFirstAllocation);
        
    const auto alignment = XM_CACHE_LINE_SIZE;
    return _aligned_malloc_dbg(size, alignment, file, line);
}

void Memory::Free(void* ptr)
{
    if (ptr)
    {
        _aligned_free_dbg(ptr);

        if (0 == InterlockedDecrement(&sAllocationCount))
            LookForMemoryLeaks();
    }
}

}
