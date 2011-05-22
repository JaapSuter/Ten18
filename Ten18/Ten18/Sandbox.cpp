#include "Ten18/Sandbox.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/Graphics/Display.h"

using namespace Ten18;

struct Vector3
{
    float X, Y, Z;
};

class WindowImpl
{
public:
    virtual bool __thiscall get_IsFullScreen() const { return false; }

    virtual void __thiscall get_Position(XMFLOAT2& ret) const { ret = mPosition; }
    virtual void __thiscall get_Size(XMFLOAT2& ret) const { ret = mSize; }

    virtual void __stdcall MakeFullScreen() {}
    
    virtual void set_Position(const XMFLOAT2& value) { mPosition = value; }
    virtual void set_Size(const XMFLOAT2& value) { mSize = value; }
    
    WindowImpl()
        : mPosition(), mSize()
    {}

private:

    XMFLOAT2 mPosition;
    XMFLOAT2 mSize;
};

extern "C" __declspec(dllexport) std::intptr_t __stdcall NewWindowImpl()
{
    auto wndImpl = new WindowImpl();
    return reinterpret_cast<std::intptr_t>(wndImpl);
}

void Ten18::Sandbox()
{
}
