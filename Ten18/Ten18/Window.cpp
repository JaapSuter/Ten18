#include "Ten18/Window.h"
#include "Ten18/Input/RawInput.h"
#include "Ten18/Assert.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/OnExit.h"
#include "Ten18/Assert.h"

namespace Ten18 {
   
static ATOM sWndClass = 0;
static volatile unsigned int sWndCount = 0;    
static int sWndProcReentrancyCount = 0;

Window::Window(const wchar_t* title)
    : mHwnd()  
{
    const auto hInstance = static_cast<HINSTANCE>(GetModuleHandle(nullptr));
        
    if (0 == sWndClass)
    {
        const auto wndClassName = L"Ten18::Window";
        WNDCLASSEX wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.lpfnWndProc = &Window::WndProc;
        wcex.hInstance = hInstance;
        wcex.lpszClassName = wndClassName;
        
        Expect.NotZero = sWndClass = RegisterClassEx(&wcex);
    }
    
    const auto style = WS_OVERLAPPEDWINDOW;
    const auto styleEx = WS_EX_OVERLAPPEDWINDOW;
    
    Expect.NotNull = mHwnd = CreateWindowEx(styleEx, MAKEINTATOM(sWndClass), title, style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, this);

    Input::RawInput::Register(mHwnd);

    Expect.Zero = ShowWindow(mHwnd, SW_SHOWDEFAULT);
    Expect.NotZero = UpdateWindow(mHwnd);

    InterlockedDecrement(&sWndCount);
}

Window::~Window()
{
    Close();
}

void Window::get_Size(XMFLOAT2& ret)
{
    RECT clientRect = {};
    Expect.True = GetClientRect(mHwnd, &clientRect);
    ret.x = static_cast<float>(clientRect.right - clientRect.left);
    ret.y = static_cast<float>(clientRect.bottom - clientRect.top);    
}

void Window::set_Size(const XMFLOAT2& value)
{    
    UNREFERENCED_PARAMETER(value);
}
    
void Window::get_Position(XMFLOAT2& ret)
{
    POINT clientPos = {};
    Expect.True = ClientToScreen(mHwnd, &clientPos);

    ret.x = static_cast<float>(clientPos.x);
    ret.y = static_cast<float>(clientPos.y);
}

void Window::set_Position(const XMFLOAT2& value)
{
    UNREFERENCED_PARAMETER(value);
}

bool Window::get_HasClosed()
{
    return mHwnd == nullptr;
}

bool Window::get_IsFullScreen()
{
    return false;
}

void Window::MakeFullScreen()
{
}

void Window::Dispose() { delete this; }

void Window::Close()
{
    if (mHwnd)
    {
        auto hwnd = mHwnd;
        mHwnd = nullptr;
        HopeFor.NotZero = CloseWindow(hwnd);

        if (0 == InterlockedDecrement(&sWndCount))
            PostQuitMessage(0);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OnExit ie([&] { --sWndProcReentrancyCount; });
    ++sWndProcReentrancyCount;

    auto thisPtr = checked_reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (thisPtr != nullptr)
        if (thisPtr->mHwnd == hwnd)
            return thisPtr->WndMethod(msg, wParam, lParam);
    
    if (WM_NCCREATE == msg)
    {   
        const auto createStruct = reinterpret_cast<const UNALIGNED LPCREATESTRUCT>(lParam);
        thisPtr = checked_reinterpret_cast<Window*>(createStruct->lpCreateParams);
        Ten18_ASSERT(thisPtr != nullptr);

        Ignore = SetWindowLongPtr(hwnd, GWLP_USERDATA, checked_reinterpret_cast<LONG_PTR>(thisPtr));
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Window::WndMethod(UINT msg, WPARAM wParam, LPARAM lParam)
{   
    Ten18_ASSERT(mHwnd != nullptr);

    if (WM_INPUT == msg)
    {
        return Input::RawInput::WndProc(mHwnd, msg, wParam, lParam);
    }
    else if (WM_ERASEBKGND == msg)
    {
        const int nonZeroIndicatesHandled = 1;
        return nonZeroIndicatesHandled;
    }
    else if (WM_PAINT == msg)
    {
        PAINTSTRUCT ps = {};
        BeginPaint(mHwnd, &ps);
        EndPaint(mHwnd, &ps);
        return 0;
    }
    else if (WM_DESTROY == msg)
    {
        Close();
        return 0;
    }
    
    return DefWindowProc(mHwnd, msg, wParam, lParam);
}

}
