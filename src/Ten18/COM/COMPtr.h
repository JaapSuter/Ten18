#pragma once
#include "Ten18/Assert.h"
#include "Ten18/Expect.h"

namespace Ten18 { namespace COM {

    template<class T>
    class COMPtr
    {
        struct MakeRawIUnknownCallsIntoCompileErrors : public T
        {
        private:
            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**);
            ULONG STDMETHODCALLTYPE AddRef();
            ULONG STDMETHODCALLTYPE Release();

            MakeRawIUnknownCallsIntoCompileErrors();
            MakeRawIUnknownCallsIntoCompileErrors(const MakeRawIUnknownCallsIntoCompileErrors&);
            MakeRawIUnknownCallsIntoCompileErrors& operator = (const MakeRawIUnknownCallsIntoCompileErrors&);
        };

        typedef void (COMPtr::*SafeBoolIdiomType)() const;
        void SafeBoolIdiomFun() const {}

    public:

        static const UUID& GetUUID() { return __uuidof(T); }

        T* Raw() const { return mPtr; }

        operator SafeBoolIdiomType() const { return mPtr == nullptr ? nullptr : &COMPtr::SafeBoolIdiomFun; }

        void Own(T* ptr)
        {
            Reset();
            mPtr = ptr;
        }

        void Reset()
        {
            T* ptr = static_cast<T*>(InterlockedExchangePointer(&mPtr, nullptr));
            if (ptr != nullptr)
                ptr->Release();
        }

        COMPtr()
            : mPtr()
        {
            static_assert(sizeof(*this) == sizeof(T*), "Error, size of smart COM pointer differs from raw interface pointer size.");
        }

        COMPtr(T* ptr)
            : mPtr(ptr)
        {
            static_assert(sizeof(*this) == sizeof(T*), "Error, size of smart COM pointer differs from raw interface pointer size.");
        }

        COMPtr(COMPtr&& rhs) : mPtr(rhs.mPtr) { rhs.mPtr = nullptr; }
        COMPtr& operator = (COMPtr&& rhs) { Reset(); mPtr = rhs.mPtr; rhs.mPtr = nullptr; return *this; }
        
        MakeRawIUnknownCallsIntoCompileErrors& operator * () const
        {
            Ten18_ASSERT(mPtr != nullptr);
            return *reinterpret_cast<MakeRawIUnknownCallsIntoCompileErrors*>(mPtr);
        }

        MakeRawIUnknownCallsIntoCompileErrors* operator -> () const
        {
            Ten18_ASSERT(mPtr != nullptr);
            return reinterpret_cast<MakeRawIUnknownCallsIntoCompileErrors*>(mPtr);
        }
        
        IUnknown** AsUnknownDoubleStar() { Ten18_ASSERT(mPtr == nullptr); return reinterpret_cast<IUnknown**>(&mPtr); }
        void**     AsVoidDoubleStar()    { Ten18_ASSERT(mPtr == nullptr); return reinterpret_cast<void**>(&mPtr); }
        T**        AsTypedDoubleStar()   { Ten18_ASSERT(mPtr == nullptr); return &mPtr; }

        T** AsUnsafeArrayOfOne() { Ten18_ASSERT(mPtr != nullptr); return &mPtr; }

        ~COMPtr() { Reset(); }

        template<class U>
        bool TryQueryInto(COMPtr<U>& dst) const
        { 
            Ten18_ASSERT(mPtr != nullptr);
            auto hr = mPtr->QueryInterface(__uuidof(U), dst.AsVoidDoubleStar());
            if (hr == E_NOINTERFACE)
                return false;
            Ten18_EXPECT.HR = hr;
            return true;
        }

        template<class U>
        void QueryInto(COMPtr<U>& dst) const
        { 
            Ten18_ASSERT(mPtr != nullptr);
            Ten18_EXPECT.True = TryQueryInto(dst);
            Ten18_ASSERT(!!dst);
        }

    private:

        COMPtr(const COMPtr&);
        COMPtr& operator= (const COMPtr&);

        T* mPtr;
    };

    void COMPtrTest();

}}
