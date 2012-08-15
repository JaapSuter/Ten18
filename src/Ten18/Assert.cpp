#include "Ten18/PCH.h"
#include "Ten18/Assert.h"
#include "Ten18/Format.h"
#include "Ten18/Util.h"
#include "Ten18/Expect.h"

bool Ten18::Assert::ReportFailure(const char* expr, const char* file, int line, const wchar_t* msg, HRESULT hr, DWORD lastErr)
{
    expr = expr ? expr : "??";
    msg = msg ? msg : L"??";
    
    Format("%0(%1): failure error: '%2': %3\n", file, line, expr, msg).DebugOut();

    if (FAILED(hr))
    {
        _com_error error(hr);
        Format("\t_com_error.ErrorMessage for HRESULT %0 gives: %1\n", hr, error.ErrorMessage()).DebugOut();
    }
    
    if (lastErr)
    {
        const auto max = 4096;
        wchar_t wbuf[max] = {};
    
        const auto len = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, lastErr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), wbuf, max, nullptr);
        Format("\nFormatMessage for last error %0 gives: %1\n", lastErr, len ? wbuf : L"Error in FormatMessage").DebugOut();
    }

    return true;
}
