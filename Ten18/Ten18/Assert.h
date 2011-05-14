#pragma once

// Based on http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert
namespace Ten18
{
    void DebugOut(const char *fmt, ...);

    struct Assert
    {
        static bool ReportFailure(HRESULT hr, DWORD lastErr, const char* expression, const char* file, int line, const char* msg, ...);
        static bool ReportFailure(const char* condition, const char* file, int line, const char* msg, ...);
    };
}

#define Ten18_BREAK() __debugbreak()
#define Ten18_UNUSED(x) do { (void)sizeof(x); } while(0)

#define Ten18_VERIFY_HR(expr) \
	do { \
        HRESULT hrUniqueEnoughAndMyBirthDayIsInApril = (expr); \
		if (FAILED((hrUniqueEnoughAndMyBirthDayIsInApril))) \
		{ \
			if (Ten18::Assert::ReportFailure(hrUniqueEnoughAndMyBirthDayIsInApril, 0, #expr, __FILE__, __LINE__, 0)) \
				Ten18_BREAK(); \
		} \
	} while(0)

#define Ten18_VERIFY_LAST_ERROR(expr) \
	do { \
        if (!(expr)) \
		{ \
			if (Ten18::Assert::ReportFailure(0, GetLastError(), #expr, __FILE__, __LINE__, 0)) \
				Ten18_BREAK(); \
		} \
	} while(0)

#ifndef Ten18_ASSERT_ENABLED
    #ifdef _DEBUG
        #define Ten18_ASSERT_ENABLED 1
    #endif
#endif

#ifdef Ten18_ASSERT_ENABLED
	#define Ten18_ASSERT(cond) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Ten18::Assert::ReportFailure(#cond, __FILE__, __LINE__, 0)) \
					Ten18_BREAK(); \
			} \
		} while(0)

	#define Ten18_ASSERT_MSG(cond, msg, ...) \
		do \
		{ \
			if (!(cond)) \
			{ \
				if (Ten18::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), __VA_ARGS__)) \
					Ten18_BREAK(); \
			} \
		} while(0)

	#define Ten18_ASSERT_FAIL(msg, ...) \
		do \
		{ \
			if (Ten18::Assert::ReportFailure(false, __FILE__, __LINE__, (msg), __VA_ARGS__)) \
			    Ten18_BREAK(); \
		} while(0)    
	
#else
	#define Ten18_ASSERT(condition) \
		do { Ten18_UNUSED(condition); } while(0)
	#define Ten18_ASSERT_MSG(condition, msg, ...) \
		do { Ten18_UNUSED(condition); Ten18_UNUSED(msg); } while(0)
	#define Ten18_ASSERT_FAIL(msg, ...) \
		do { Ten18_UNUSED(msg); } while(0)	
#endif
