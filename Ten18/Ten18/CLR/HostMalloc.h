#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/Scoped.h"

namespace Ten18 { namespace CLR {

    class HostMalloc : public IHostMalloc 
    {
    public:
        explicit HostMalloc(DWORD mallocType);
        HostMalloc(HostMalloc&& other);

        Ten18_QUERY_INTERFACE_BEGIN(IHostMalloc)
            Ten18_QUERY_INTERFACE_IMPL(IHostMalloc)
        Ten18_QUERY_INTERFACE_END()

        virtual HRESULT STDMETHODCALLTYPE Alloc(SIZE_T cbSize, EMemoryCriticalLevel eCriticalLevel, void** ppMem);
        virtual HRESULT STDMETHODCALLTYPE Free(void* pMem);
        virtual HRESULT STDMETHODCALLTYPE DebugAlloc(SIZE_T cbSize, EMemoryCriticalLevel eCriticalLevel,
            char* pszFileName, int iLineNo, void** ppMem);

    private:

        HostMalloc(const HostMalloc&);
        HostMalloc& operator = (const HostMalloc&);

        Scoped<HANDLE> mHeap;
    };
}}
