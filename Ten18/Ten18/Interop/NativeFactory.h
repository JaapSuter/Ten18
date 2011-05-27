#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 { namespace Interop {

class NativeFactory
{
public:
    #include "Ten18/Interop/NativeFactory.Generated.h"

    ~NativeFactory() {};        
};

}}