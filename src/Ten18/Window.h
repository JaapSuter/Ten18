#pragma once

#include "Ten18/Memory.h"
#include "Ten18/Graphics/SwapChain.h"

namespace Ten18 {

class Window
{
    Ten18_CUSTOM_OPERATOR_NEW_DELETE
public:
    explicit Window(const wchar_t* title);
    ~Window();
    
    HWND Handle() const { return mHwnd; }

    void get_Size(dx::XMFLOAT2& ret) const;
    void set_Size(const dx::XMFLOAT2& value);
    void get_Position(dx::XMFLOAT2& ret) const;
    void set_Position(const dx::XMFLOAT2& value);
    bool get_HasClosed() const;
    bool get_IsFullScreen() const;
    void MakeFullScreen();

private:

    Window(const Window&);
    Window& operator = (const Window&);
    
    void Close();

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WndMethod(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND mHwnd;
};

}
