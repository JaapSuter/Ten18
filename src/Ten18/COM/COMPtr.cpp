#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/Memory.h"

namespace Ten18 { namespace COM {

class __declspec(uuid("6a529c0d-1018-1018-1018-29b014e96680")) ICOMPtrTestInterface : public IUnknown
{
public:
    virtual int STDMETHODCALLTYPE Return1018() = 0;
};

class COMPtrTestClass : public ICOMPtrTestInterface
{
    Ten18_CUSTOM_OPERATOR_NEW_DELETE
public:

    Ten18_QUERY_INTERFACE_BEGIN(ICOMPtrTestInterface)
        Ten18_QUERY_INTERFACE_IMPL(ICOMPtrTestInterface)
    Ten18_QUERY_INTERFACE_END()
        
    static ULONG ObjCount() { return sObjCount; }

    ULONG RefCount() const { return mRefCount.Current(); }
        
    COMPtrTestClass(bool stackBased = false)
        : mRefCount(stackBased)
    {
        InterlockedIncrement(&sObjCount);
    }

    virtual int STDMETHODCALLTYPE Return1018()
    {
        return 1018;
    }

    ~COMPtrTestClass()
    {
        InterlockedDecrement(&sObjCount);        
    }

private:
    COMPtrTestClass(const COMPtrTestClass&);
    COMPtrTestClass& operator = (const COMPtrTestClass&);

    static ULONG sObjCount;
};

ULONG COMPtrTestClass::sObjCount = 0;

static void CreateIntoVoidDoubleStar(void** ptr)
{
    Ten18_ASSERT(ptr != nullptr);
    *ptr = Ten18_NEW COMPtrTestClass();
}

static void CreateIntoTypedDoubleStar(ICOMPtrTestInterface** ptr)
{
    Ten18_ASSERT(ptr != nullptr);
    *ptr = Ten18_NEW COMPtrTestClass();
}

void COMPtrTest()
{
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 0);
    auto stackBased = true;
    COMPtrTestClass obj(stackBased);
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
    Ten18_ASSERT(1 == obj.RefCount());
    obj.Release();
    Ten18_ASSERT(0 == obj.RefCount());

    ICOMPtrTestInterface* raw;
    CreateIntoVoidDoubleStar(reinterpret_cast<void**>(&raw));
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 2);
    auto rc = raw->Release();
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
    Ten18_ASSERT(rc == 0);

    raw = nullptr;
    CreateIntoTypedDoubleStar(&raw);
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 2);
    rc = raw->Release();
    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
    Ten18_ASSERT(rc == 0);

    {
        Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
        COMPtr<ICOMPtrTestInterface> ptr;
        Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
        CreateIntoVoidDoubleStar(ptr.AsVoidDoubleStar());
        Ten18_ASSERT(COMPtrTestClass::ObjCount() == 2);
        Ten18_ASSERT(1018 == ptr->Return1018());

        // These should give a compile error. Ideally I could test this 
        // properly, but it'd require build infrastructure I do
        // not have right now.
        //
        //      ptr->AddRef();
        //      ptr->Release();
        //      ptr->QueryInterface(__uuidof(IUnknown), nullptr);
    }

    Ten18_ASSERT(COMPtrTestClass::ObjCount() == 1);
}

}}
