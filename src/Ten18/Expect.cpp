#include "Ten18/PCH.h"
#include "Ten18/Expect.h"
#include "Ten18/Assert.h"

namespace Ten18 {

ExpectImpl Expect;
ExpectImpl HopeFor;
IgnoreImpl Ignore;

__declspec(thread) const char* ExpectImpl::tSourceFile = nullptr;
__declspec(thread) int ExpectImpl::tLineNumber = 0;

}
