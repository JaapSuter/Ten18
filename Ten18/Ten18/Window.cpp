#include "Ten18/Window.h"
#include "Ten18/Assert.h"

namespace Ten18 {

class Foo : public Window {

Foo();
};

Window::Window(const char16_t*, int width, int height)
    : mPosition(), mSize(static_cast<float>(width), static_cast<float>(height))
{   
}

void Window::get_Size(XMFLOAT2& ret) { ret = mSize; }
void Window::set_Size(const XMFLOAT2& value) { mSize = value; }
    
void Window::get_Position(XMFLOAT2& ret) { ret = mPosition; }
void Window::set_Position(const XMFLOAT2& value) { mPosition = value; }

bool Window::get_IsFullScreen() { return false; }
void Window::MakeFullScreen()
{
}

void Window::Dispose()
{
    delete this;
}

extern "C" void __declspec(dllexport) __stdcall PeeInvokeDeleteWindow(int wnd)
{
    delete reinterpret_cast<Foo*>(wnd);
}

extern "C" int __declspec(dllexport) __stdcall PeeInvokeNewWindow(const char16_t* title, int w, int h)
{
    return reinterpret_cast<int>(new Window(title, w, h));
}

extern "C" void __declspec(dllexport) __stdcall PeeInvokeSet(int w, XMFLOAT2& v)
{
    return reinterpret_cast<Foo*>(w)->set_Position(v);
}

extern "C" void __declspec(dllexport) __stdcall PeeInvokeGet(int w, XMFLOAT2& ret)
{
    reinterpret_cast<Foo*>(w)->get_Position(ret);
}

}
