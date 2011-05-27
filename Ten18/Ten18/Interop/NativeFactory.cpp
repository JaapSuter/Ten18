#include "Ten18/Interop/NativeFactory.h"
#include "Ten18/Window.h"

namespace Ten18 { namespace Interop {
    
void* NativeFactory::NewInput()
{
    return nullptr;
}

void* NativeFactory::NewWindow(std::int32_t i, bool b)
{
    return new Ten18::Window(i, b);
}

}}
