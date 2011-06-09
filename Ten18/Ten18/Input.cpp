#include "Ten18/Input.h"

namespace Ten18 {

Input::Input()
{
}

void* Input::New()
{
    return new Input();
}

void __thiscall Input::get_MousePosition(XMFLOAT2& ret)
{
    ret.x = ret.y = 0.0f;
}

}
