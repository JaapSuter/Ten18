#include "Ten18/Interop/NativeFactory.h"
#include "Ten18/Window.h"

namespace Ten18 { namespace Interop {
    
void* NativeFactory::NewInput()
{
    return nullptr;
}

void* NativeFactory::NewWindow(const wchar_t* title)
{
    return new Ten18::Window(title);
}

}}
