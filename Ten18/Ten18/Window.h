#pragma once

namespace Ten18 {

class Window
{
public:
    Window(std::int32_t i, bool b);

    #include "Ten18/Window.Generated.h"

private:

    ~Window() {}

    XMFLOAT2 mSize;
    XMFLOAT2 mPosition;
};

}
