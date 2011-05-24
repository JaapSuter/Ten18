#pragma once

namespace Ten18 {

class Window
{
public:
    Window();

    #include "Ten18/Window.Interop.Generated.h"

private:

    ~Window() {}

    XMFLOAT2 mSize;
    XMFLOAT2 mPosition;
};

}
