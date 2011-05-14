#pragma once
#include "Ten18/Assert.h"

namespace Ten18 { namespace COM {

//--------------------------------------------------------------------------------------
struct RefCount
{
    RefCount() : mRefCount(1), mStackBased() {}
    explicit RefCount(bool stackBased) : mRefCount(1), mStackBased(stackBased) {}

    ULONG AddRef() { Ten18_ASSERT(mRefCount > 0); InterlockedIncrement(&mRefCount); return mRefCount; }
    ULONG Release() { Ten18_ASSERT(mRefCount > 0); return InterlockedDecrement(&mRefCount); }
    ULONG Current() const { return mRefCount; }

    bool IsStackBased() const { return mStackBased; }

private:
    RefCount(const RefCount&);
    RefCount& operator= (const RefCount&);

    ULONG mRefCount;
    bool mStackBased;
};

#define Ten18_QUERY_INTERFACE_BEGIN(leftMostInterfaceToDisambiguateIUnknown) \
    ::Ten18::COM::RefCount mRefCount; \
    ULONG STDMETHODCALLTYPE RefCount() { return mRefCount.Current(); } \
    virtual ULONG STDMETHODCALLTYPE AddRef() { return mRefCount.AddRef(); } \
    virtual ULONG STDMETHODCALLTYPE Release() \
    { \
        auto refCount = mRefCount.Release(); \
        if (refCount == 0) \
            if (!mRefCount.IsStackBased()) \
                delete this; \
        return refCount; \
    } \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppObject) \
    { \
        if (nullptr == ppObject) \
            return E_INVALIDARG; \
        if (riid == IID_IUnknown) \
        { \
            *ppObject = static_cast<IUnknown*>(static_cast<leftMostInterfaceToDisambiguateIUnknown*>(this)); \
            AddRef(); \
            return NOERROR; \
        }

#define Ten18_QUERY_INTERFACE_IMPL(i)           \
    if (riid == __uuidof(i))                    \
    {                                           \
        *ppObject = static_cast<i*>(this);      \
        AddRef();                               \
        return NOERROR;                         \
    }
        
#define Ten18_QUERY_INTERFACE_END() \
        *ppObject = nullptr; \
        return E_NOINTERFACE;  \
    }

}}
