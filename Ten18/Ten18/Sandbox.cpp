#include "Ten18/Sandbox.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"
#include "Ten18/Graphics/Display.h"

using namespace Ten18;

class ISomeInterface
{
public:
    virtual ~ISomeInterface() = 0 {}
    virtual void SomeFunc() = 0;
};

class IOther2
{
public:

    virtual ~IOther2() = 0 {}
    virtual void SomeFunc() = 0;

    IOther2* Make()
    {
    }

    static const int NumMethods = 2;
    static std::intptr_t sVTable[NumMethods];
};

std::intptr_t IOther2::sVTable[NumMethods] = {};

class SomeInterfaceImpl : public ISomeInterface
{
public:
    SomeInterfaceImpl() {}
    virtual ~SomeInterfaceImpl() {}
    virtual void SomeFunc()
    {
        DebugOut("Hello World\n");
    };

private:
    SomeInterfaceImpl(const SomeInterfaceImpl&);
    SomeInterfaceImpl& operator = (const SomeInterfaceImpl&);
};

void Ten18::Sandbox()
{
    ISomeInterface* ptr = new SomeInterfaceImpl();
    
    ptr->SomeFunc();

    auto vTablePtr = checked_reinterpret_cast<std::intptr_t**>(ptr);
    IOther2::sVTable[0] = (*vTablePtr)[0];
    IOther2::sVTable[1] = (*vTablePtr)[1];   

    auto vTablePtr2 = checked_reinterpret_cast<std::intptr_t>(&IOther2::sVTable[0]);
    auto rawThisPtr = checked_reinterpret_cast<std::intptr_t>(&vTablePtr2);

    auto fakeThis = checked_reinterpret_cast<IOther2*>(rawThisPtr);

    fakeThis->SomeFunc();
    
    delete ptr;
}
