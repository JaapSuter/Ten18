#include "Ten18/PCH.h"
#include "Ten18/Assert.h"
#include "Ten18/Util.h"
#include "Ten18/Expect.h"

#ifdef _DEBUG
    #include <dxerr.h>
#else
    #define DXGetErrorStringA(hr) "";
#endif

void Ten18::DebugOut(const char *fmt, ...)
{
    const int len = 1024;
    char buf[len + 1] = {};
    char* p = buf;
    std::va_list args;
    
    va_start(args, fmt);
    int n = _vsnprintf_s(p, sizeof(buf) - 3, _TRUNCATE, fmt, args);    
    va_end(args);

    p += (n < 0) ? sizeof buf - 3 : n;

    while (p > buf  &&  isspace(p[-1]))
        *--p = '\0';

    *p++ = '\r';
    *p++ = '\n';
    *p   = '\0';

    OutputDebugStringA(buf);
}

bool Ten18::Assert::ReportFailure(HRESULT hr, DWORD lastErr, const char* expression, const char* file, int line, const char* fmt, ...)
{
    const auto len = 1024;
    char buf[len] = {};
    wchar_t wbuf[len] = {};
    		
    if (fmt != nullptr)
	{
        std::va_list args;
		va_start(args, fmt);
        _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
		va_end(args);
	}

    if (hr == 0 && lastErr != 0)
    {
        auto rslt = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), wbuf, len, nullptr);
        if (rslt)
            DebugOut("FormatMessage from system gives: %S\n", wbuf);

        DebugOut("%s(%d): %s Failed With %S(0x%08x): '%S' %s\n", file, line, expression, L"GetLastError", lastErr, wbuf, buf);
    }
    else
    {
        _com_error ce(hr);
        auto dxe = DXGetErrorString(hr);
        DXTRACE_ERR(dxe, hr);
        DebugOut("%s(%d): %s Failed With %S(0x%08x): '%S' %s\n", file, line, expression, dxe, hr, ce.ErrorMessage(), buf);
    }

    return true;
}

bool Ten18::Assert::ReportFailure(const char* condition, const char* file, int line, const char* fmt, ...)
{
    const auto len = 1024;
    char buf[len] = {};
    		
    if (fmt != nullptr)
	{
        std::va_list args;
		va_start(args, fmt);
        _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
		va_end(args);
	}
    
    DebugOut("%s(%d): Assert Failure: '%s' %s\n", file, line, condition, buf);

	return true;
}

