#pragma once

#include "Ten18/Memory.h"
#include "Ten18/Graphics/SwapChain.h"

namespace Ten18 {

class Window
{
    Ten18_CUSTOM_OPERATOR_NEW_DELETE
public:
    explicit Window(const wchar_t* title);

    #include "Ten18/Window.Generated.h"

    HWND Handle() const { return mHwnd; }

private:

    ~Window();
    Window(const Window&);
    Window& operator = (const Window&);

    static HWND Create(Window& dst, const wchar_t* title);

    void Close();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WndMethod(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND mHwnd;
    Graphics::SwapChain mSwapChain;
};

}
