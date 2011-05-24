#include "Ten18/Interop/NativeTypeFactory.h"
#include "Ten18/Window.h"

namespace Ten18 { namespace Interop {
    
std::int32_t NativeTypeFactory::CreateNativeInput()
{
    return 0;
}

std::int32_t NativeTypeFactory::CreateNativeWindow()
{
    return reinterpret_cast<std::intptr_t>(new Window());
}

}}
