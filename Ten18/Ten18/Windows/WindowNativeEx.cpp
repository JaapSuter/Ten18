#include "Ten18/PCH.h"
#include "Ten18/Windows/Window.h"
#include "Ten18/Windows/RawInput.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/OnExit.h"
#include "Ten18/Assert.h"

using namespace Ten18::Windows;
using namespace Ten18;

namespace
{
    ATOM sWndClass = 0;
    volatile unsigned int sWndCount = 0;    
    int sWndProcReentrancyCount = 0;
}

Window::Window(const wchar_t* title)
  : mHwnd(),
    mOnRender(),
    mOnClosed(),
    mUndoToggleWindowedFullScreen()
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

    RawInput::Register(mHwnd);

    InterlockedIncrement(&sWndCount);
}

void Window::Show(int nCmdShow)
{
    Expect.Zero = ShowWindow(mHwnd, nCmdShow);
    Expect.NotZero = UpdateWindow(mHwnd);
}

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
        if (thisPtr->Handle() == hwnd)
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
        return RawInput::WndProc(mHwnd, msg, wParam, lParam);
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

void Window::Repose(const RECT& monitor, int width, int height)
{
    auto style = GetWindowLongPtr(mHwnd, GWL_STYLE);
    auto styleEx = GetWindowLongPtr(mHwnd, GWL_EXSTYLE);
    POINT clientPos = {};
    Expect.True = ClientToScreen(mHwnd, &clientPos);
    
    RECT windowRect = { monitor.left + clientPos.x, monitor.top + clientPos.y, monitor.left + clientPos.x + width, monitor.top + clientPos.y + height };
    Expect.True = AdjustWindowRectEx(&windowRect, style, FALSE, styleEx);

    Expect.True = SetWindowPos(mHwnd, HWND_TOP,
            windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_FRAMECHANGED);
}

void Window::ToggleWindowedFullScreen()
{
    if (mUndoToggleWindowedFullScreen)
        mUndoToggleWindowedFullScreen();
    else
    {
        RECT windowRect = {};
        Expect.True = GetWindowRect(mHwnd, &windowRect);
        auto style = GetWindowLongPtr(mHwnd, GWL_STYLE);
        auto styleEx = GetWindowLongPtr(mHwnd, GWL_EXSTYLE);
        
        mUndoToggleWindowedFullScreen = [=]
        {
            Expect.True = SetWindowLongPtr(mHwnd, GWL_STYLE, style);
            Expect.True = SetWindowLongPtr(mHwnd, GWL_EXSTYLE, styleEx);
            Expect.True = SetWindowPos(mHwnd, nullptr, windowRect.left, windowRect.top,
                windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,  SWP_FRAMECHANGED);

            mUndoToggleWindowedFullScreen = nullptr;
        };
                
        auto hmon = MonitorFromWindow(mHwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {};
        mi.cbSize = sizeof(mi);

        Expect.True = GetMonitorInfo(hmon, &mi);

        const auto remove = ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
        Expect.True = SetWindowLongPtr(mHwnd, GWL_STYLE, remove & style);

        const auto removeEx = ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        Expect.True = SetWindowLongPtr(mHwnd, GWL_EXSTYLE, removeEx & styleEx);

        Expect.True = SetWindowPos(mHwnd, HWND_TOP,
            mi.rcMonitor.left, mi.rcMonitor.top,
            mi.rcMonitor.right - mi.rcMonitor.left,
            mi.rcMonitor.bottom - mi.rcMonitor.top,
            SWP_FRAMECHANGED);
    }
}

/*
    else if (WM_WINDOWPOSCHANGED == msg)
    {
        if (mOnRender)
            mOnRender(*this);

    }

    UINT_PTR modalTimerId = 0;
    if (WM_SYSCOMMAND == msg)
        if (SC_MOVE == GET_SC_WPARAM(wParam) || SC_SIZE == GET_SC_WPARAM(wParam))
        {
            modalTimerId = SetTimer(mHwnd, checked_reinterpret_cast<UINT_PTR>(this), USER_TIMER_MINIMUM, &TimerProc);
            Expect.EqualTo(reinterpret_cast<UINT_PTR>(this)) = modalTimerId;
        }

    if (modalTimerId != 0)
    {        
        Ten18_ASSERT(modalTimerId == reinterpret_cast<UINT_PTR>(this));
        Expect.True = KillTimer(mHwnd, modalTimerId);
        modalTimerId = 0;
    }

*/