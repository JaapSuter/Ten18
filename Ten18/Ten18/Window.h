#pragma once

namespace Ten18 {

class Window
{
public:
    Window(const char16_t* title, int width, int height);

    #include "Ten18/Window.Generated.h"

private:
    
    XMFLOAT2 mSize;
    XMFLOAT2 mPosition;
};

}
