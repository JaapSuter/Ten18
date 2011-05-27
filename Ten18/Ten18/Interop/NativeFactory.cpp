#include "Ten18/Interop/NativeFactory.h"
#include "Ten18/Window.h"

namespace Ten18 { namespace Interop {
    
void* NativeFactory::NewInput()
{
    return nullptr;
}

void* NativeFactory::NewWindow(const char16_t* title, int w, int h)
{
    return new Ten18::Window(title, w, h);
}

}}
