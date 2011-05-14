#include "Ten18/PCH.h"
#include "Ten18/CLR/HostMalloc.h"
#include "Ten18/Resources/Resources.h"
#include "Ten18/Expect.h"
#include "Ten18/COM/StackBasedSafeArray.h"
#include "Ten18/COM/EmbeddedResourceStream.h"

using namespace Ten18;
using namespace Ten18::CLR;
using namespace Ten18::COM;

static DWORD MallocTypeToHeapFlags(DWORD mallocType)
{
    DWORD heapFlags = 0;
    if (  mallocType & MALLOC_EXECUTABLE)  heapFlags |= HEAP_CREATE_ENABLE_EXECUTE;
    if (!(mallocType & MALLOC_THREADSAFE)) heapFlags |= HEAP_NO_SERIALIZE;
    return heapFlags;
}

HostMalloc::HostMalloc(DWORD mallocType)
    : mHeap(HeapCreate(MallocTypeToHeapFlags(mallocType), 8 * 1024 * 1024, 0), [&] (HANDLE h)
            {
                if (h)
                    HopeFor.NotZero = HeapDestroy(h); 
            })
{
}

HostMalloc::HostMalloc(HostMalloc&& other)
    : mHeap(std::move(other.mHeap))
{}

HRESULT STDMETHODCALLTYPE HostMalloc::Alloc(SIZE_T cbSize, EMemoryCriticalLevel, void** ppMem)
{
    Ten18_ASSERT(ppMem);
    if (!ppMem)
        return E_FAIL;
    *ppMem = HeapAlloc(mHeap, 0, cbSize);
    return *ppMem ? S_OK : E_OUTOFMEMORY;
}

HRESULT STDMETHODCALLTYPE HostMalloc::Free(void* pMem)
{
    // Todo, Jaap Suter, April 2011, from MSDN: If the pMem parameter refers to a 
    // region of memory that was not allocated by using a call to Alloc, the host should
    // return HOST_E_INVALIDOPERATION.
    if (pMem)
        HopeFor.NotZero = HeapFree(mHeap, 0, pMem);    
    return S_OK;
}

HRESULT STDMETHODCALLTYPE HostMalloc::DebugAlloc(SIZE_T cbSize, EMemoryCriticalLevel, char*, int, void** ppMem)
{
    Ten18_ASSERT(ppMem);
    if (!ppMem)
        return E_FAIL;
    *ppMem = HeapAlloc(mHeap, 0, cbSize);
    return *ppMem ? S_OK : E_OUTOFMEMORY;
}

