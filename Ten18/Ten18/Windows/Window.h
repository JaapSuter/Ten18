#pragma once
#include "Ten18/PCH.h"

namespace Ten18 { namespace Windows {

    class Window
    {
    public:

        typedef std::function<void (const Window&)> EventCallback;
        
        explicit Window(const wchar_t* title);
        
        void Show(int nCmdShow = SW_SHOW);
        void Close();

        void OnRenderDo(const EventCallback& ec) { mOnRender = ec; }
        void OnClosedDo(const EventCallback& ec) { mOnClosed = ec; }

        HWND Handle() const { return mHwnd; }
        
        void ToggleWindowedFullScreen();
        void Repose(const RECT& monitor, int width, int height);

    private:
        
        Window(const Window&);
        Window& operator = (const Window&);
        
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT CALLBACK WndMethod(UINT msg, WPARAM wParam, LPARAM lParam);

        HWND mHwnd;

        EventCallback mOnRender;
        EventCallback mOnClosed;
        std::function<void ()> mUndoToggleWindowedFullScreen;
    };
}}
