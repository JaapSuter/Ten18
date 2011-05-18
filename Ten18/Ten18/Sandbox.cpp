#include "Ten18/Sandbox.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/Graphics/Display.h"

using namespace Ten18;

class WindowImplBase
{
public:
    virtual bool __stdcall get_IsFullScreen() const { return false; }

    virtual float __stdcall get_Position() const { return 3.14f; }
    virtual int __stdcall get_Size() const { return 42; }

    virtual void __stdcall MakeFullScreen() {}
    
    // virtual void set_Position(const XMFLOAT2&) {}
    // virtual void set_Size(const XMFLOAT2&) {}
    
    virtual void __stdcall set_Position(float) {}
    virtual void __stdcall set_Size(int) {}

    WindowImplBase()
        : mChar('q'), mInt(42), mFloat(3.14f)
    {
        DebugOut("WindowImpl::Ctor");
    }

    virtual int __stdcall Z_StdCallFoo(int, int, int) { return 0; }
    virtual int __thiscall Z_ThisCallFoo(int, int, int) { return 0; }

private:

    char mChar;
    int mInt;
    float mFloat;
};

class WindowImpl : public WindowImplBase
{
public:
    virtual bool __stdcall get_IsFullScreen() const { return false; }

    virtual float __stdcall get_Position() const { return 3.14f; }
    virtual int __stdcall get_Size() const { return 42; }

    virtual void __stdcall MakeFullScreen() {}
    
    // virtual void set_Position(const XMFLOAT2&) {}
    // virtual void set_Size(const XMFLOAT2&) {}
    
    virtual void __stdcall set_Position(float) {}
    virtual void __stdcall set_Size(int) {}

    WindowImpl()
        : mChar('q'), mInt(42), mFloat(3.14f)
    {
        DebugOut("WindowImpl::Ctor");
    }

    virtual int __stdcall Z_StdCallFoo(int a0, int a1, int a2)
    {
        // DebugOut("WindowImpl::Zoobar(%i)=[%c, %d, %f]", param, mChar, mInt, mFloat);
        return (mInt + a0) * a1 - a2;
    }

    virtual int __thiscall Z_ThisCallFoo(int a0, int a1, int a2)
    {
        // DebugOut("WindowImpl::Zoobar(%i)=[%c, %d, %f]", param, mChar, mInt, mFloat);
        return (mInt + a0) * a1 - a2;
    }

private:

    char mChar;
    int mInt;
    float mFloat;
};

extern "C" __declspec(dllexport) std::intptr_t __stdcall NewWindowImpl()
{
    DebugOut("sizeof(XMFLOAT3A) = %d", sizeof(XMFLOAT3A));
    DebugOut("sizeof(XMFLOAT3)  = %d", sizeof(XMFLOAT3));

    auto wndImpl = new WindowImpl();
    return reinterpret_cast<std::intptr_t>(wndImpl);
}

void Ten18::Sandbox()
{
    WindowImpl wi;
    WindowImplBase& w = wi;

    int i0 = w.Z_StdCallFoo(1, 2, 3);
    int i1 = w.Z_ThisCallFoo(100, 200, 300);

    DebugOut("%i, %i", i0, i1);
}

