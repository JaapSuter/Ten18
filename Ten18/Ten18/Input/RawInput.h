#pragma once
#include "Ten18/PCH.h"

namespace Ten18 { namespace Input {
    
    class RawInput
    {
    public:
        static void Register(HWND hwnd);
        static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    private:
        RawInput();
    };
}}
