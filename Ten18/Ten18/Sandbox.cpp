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

    virtual Vector3 __fastcall Z_ThisCallFoo(int a0, const Vector3& a1, int a2)
    {
        Vector3 r3 = { (mInt + a0) * a1.X + a1.Y + a1.Z - a2, 0.333f, 0.444f };
        return r3;
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
    
    Vector3 v3 = { 1, 2, 3 };
    auto i0 = wi.Z_ThisCallFoo(0xF1, v3, 0xF3);
    
    DebugOut("(%f, %f, %f)", i0.X, i0.Y, i0.Y); 
}
