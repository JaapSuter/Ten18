#pragma once

#include "Ten18/Memory.h"
#include "Ten18/Util.h"
#include "Ten18/Expect.h"
#include "Detours Express 3.0/include/detours.h"

namespace Ten18 {

std::wstring GetStackWalk()
{
    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME );
    Ten18_EXPECT.True = SymInitialize(GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", TRUE);

    const DWORD numFramesToSkip = 1;
    const DWORD numFramesToGrab = 8;
    PVOID addrs[numFramesToGrab] = {};
    const int numFrames = CaptureStackBackTrace(numFramesToSkip, numFramesToGrab, addrs, nullptr);

    std::wstring result;

    const auto currentProcess = GetCurrentProcess();

    for (auto i = 0; i < numFrames; ++i)
    {
        union SymbolInfo {
            const enum { MaxNameLen = 256 };
        
            SYMBOL_INFO mSymbolInfo;
            char mNameData[sizeof(SYMBOL_INFO) + MaxNameLen + 1];
            ULONG64 mAligner;

            #pragma warning(disable: 4351)
            SymbolInfo() 
                : mNameData()
            {
                mSymbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);
                mSymbolInfo.MaxNameLen = MaxNameLen;
            }
        };

        DWORD displacement32 = 0;
        DWORD64 displacement64 = 0;
        DWORD_PTR frame = checked_reinterpret_cast<DWORD_PTR>(addrs[i]);

        SymbolInfo symbolInfo;
        const auto haveSymbol = SymFromAddr(currentProcess, frame, &displacement64, &symbolInfo.mSymbolInfo);

        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        const auto haveSourceFileAndLine = SymGetLineFromAddr64(currentProcess, frame, &displacement32, &line);

        if (haveSourceFileAndLine)
            result.append(Format("%0(%1): ", line.FileName, line.LineNumber).str());
        else
            result.append(L"__unknown_source_file__(??): ");
        
        if (haveSymbol)
            result.append(Format("%0", std::string(symbolInfo.mSymbolInfo.Name, symbolInfo.mSymbolInfo.NameLen)).str());

        result.append(L"\n");
    }

    SymCleanup(currentProcess);

    return result;
}

static volatile long sAllocationCount = 0;
static _CrtMemState sCrtMemStateAtFirstAllocation = {};

static void LookForMemoryLeaks()
{
    #ifdef Ten18_DEBUG
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
                Ten18_EXPECT.Zero = _CrtDumpMemoryLeaks();
            }
        }
    #endif
}

typedef HANDLE (WINAPI *Type_HeapCreate)(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize);
typedef BOOL (WINAPI *Type_HeapDestroy)(HANDLE hHeap);
typedef LPVOID (WINAPI *Type_HeapAlloc)(HANDLE hHeap, DWORD dwFlags, DWORD_PTR dwBytes);
typedef LPVOID (WINAPI *Type_HeapReAlloc)(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
typedef BOOL (WINAPI *Type_HeapFree)(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);    
    
struct Hooked
{
    static LPVOID WINAPI HeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD_PTR dwBytes)
    {
        return sHeapAlloc(hHeap, dwFlags, dwBytes);
    }

    static decltype(&::HeapAlloc) sHeapAlloc;
};

decltype(&::HeapAlloc) Hooked::sHeapAlloc = &::HeapAlloc;

void Memory::Initialize()
{
    // Ten18_EXPECT.EqualTo(NO_ERROR) = DetourTransactionBegin();
    // Ten18_EXPECT.EqualTo(NO_ERROR) = DetourUpdateThread(GetCurrentThread());
    // Ten18_EXPECT.EqualTo(NO_ERROR) = DetourAttach(checked_reinterpret_cast<PVOID*>(&Hooked::sHeapAlloc), Hooked::HeapAlloc);
    // Ten18_EXPECT.EqualTo(NO_ERROR) = DetourTransactionCommit();

    Ten18_EXPECT.Not(-1) = _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    Ten18_EXPECT.Not(-1) = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    Ten18_EXPECT.Not(-1) = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);

    Ten18_EXPECT.Not(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    Ten18_EXPECT.Not(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    Ten18_EXPECT.Not(_CRTDBG_HFILE_ERROR) = _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    const auto crtDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    const auto addDbgFlags = _CRTDBG_CHECK_EVERY_128_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF;
    Ten18_EXPECT.EqualTo(crtDbgFlags) = _CrtSetDbgFlag(crtDbgFlags | addDbgFlags);
    (void)addDbgFlags;
}

void Memory::Shutdown()
{
    Ten18_ASSERT_FMT(0 == sAllocationCount, "Memory leaks: %0 allocations have not been freed.", sAllocationCount);
    
    LookForMemoryLeaks();
        
}

void* Memory::Alloc(std::size_t size, const char* file, int line)
{
    Ten18_UNUSED(file);
    Ten18_UNUSED(line);

    const long first = 1;
    if (first == InterlockedIncrement(&sAllocationCount))
        _CrtMemCheckpoint(&sCrtMemStateAtFirstAllocation);
        
    static_assert(std::alignment_of<dx::XMVECTOR>::value == 16, "Unexpected dx::XMVECTOR alignment");
    const auto alignment = std::alignment_of<dx::XMVECTOR>::value;
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
