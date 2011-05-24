#include "Ten18/Window.h"

namespace Ten18 {

Window::Window()
    : mPosition(), mSize()
{}

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

}
