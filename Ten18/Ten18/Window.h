#pragma once

#include "Ten18/Interop/Boolean.h"

namespace Ten18 {

class Window
{
public:
    explicit Window(const wchar_t* title);

    #include "Ten18/Window.Generated.h"

    bool __declspec(dllexport) __thiscall ExportedFunc();

    HWND Handle() const { return mHwnd; }

private:

    ~Window();
    Window(const Window&);
    Window& operator = (const Window&);

    void Close();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WndMethod(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND mHwnd;    
};

}
