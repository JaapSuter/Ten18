#include "Ten18/Sandbox.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/Graphics/Display.h"

using namespace Ten18;

class WindowImpl
{
public:
    virtual void __thiscall get_Size(XMFLOAT2& ret) const { ret = mSize; }
    virtual void __thiscall set_Size(const XMFLOAT2& value) { mSize = value; }
    
    virtual void __thiscall get_Position(XMFLOAT2& ret) const { ret = mPosition; }
    virtual void __thiscall set_Position(const XMFLOAT2& value) { mPosition = value; }

    virtual bool __thiscall get_IsFullScreen() const { return false; }
    virtual void __thiscall MakeFullScreen() {}

    WindowImpl()
        : mPosition(), mSize()
    {}

private:

    XMFLOAT2 mPosition;
    XMFLOAT2 mSize;
};

class NativeTypeFactory
{
    virtual std::intptr_t __thiscall CreateNativeInput()
    {
        return 0;
    }

    virtual std::intptr_t __thiscall CreateNativeWindow()
    {
        auto wndImpl = new WindowImpl();
        return reinterpret_cast<std::intptr_t>(wndImpl);
    }

	virtual __stdcall ~NativeTypeFactory() {};
};

extern "C" std::intptr_t gNativeTypeFactory = reinterpret_cast<std::intptr_t>(new NativeTypeFactory());

void Ten18::Sandbox()
{
}
