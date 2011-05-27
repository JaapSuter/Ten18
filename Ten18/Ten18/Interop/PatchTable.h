#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 { namespace Interop {

class PatchTable
{
public:
    static void Update(void* data, int size);
};

}}