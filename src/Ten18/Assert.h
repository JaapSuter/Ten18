#pragma once

#include "Ten18/Format.h"

// Based on http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert
namespace Ten18
{
    #ifdef Ten18_DEBUG    
        inline void DebugOut(const char* s) { OutputDebugStringA(s); }
        inline void DebugOut(const wchar_t* s) { OutputDebugStringW(s); }
    #else
        #define DebugOut __noop
    #endif
    
    struct Assert
    {
        static bool ReportFailure(const char* expr, const char* file, int line, const wchar_t* msg, HRESULT hr = S_OK, DWORD lastErr = 0);
    };
}

#define Ten18_UNUSED(x) _CRT_UNUSED(x)

#ifdef Ten18_DEBUG
    #define Ten18_ASSERT_FMT(expr, fmt, ...) __analysis_assume(!!(expr)); (void) ((!!(expr)) \
        || (!::Ten18::Assert::ReportFailure(#expr, __FILE__, __LINE__, ::Ten18::Format(fmt, __VA_ARGS__).c_str())) \
        || (__debugbreak(), 0))

    #define Ten18_ASSERT(expr) Ten18_ASSERT_FMT(expr, "??")

    #define Ten18_FAIL_FMT(fmt, ...) Ten18_ASSERT_FMT(false && "Ten18_FAIL_FMT", fmt, __VA_ARGS__)
#else
	#define Ten18_ASSERT(expr) __noop
	#define Ten18_ASSERT_FMT(expr, msg, ...) __noop
	#define Ten18_FAIL_FMT(msg, ...) __noop
#endif
