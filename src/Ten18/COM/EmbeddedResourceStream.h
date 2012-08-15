#pragma once
#include "Ten18/COM/ImplUnknown.h"

namespace Ten18 { namespace COM {

//--------------------------------------------------------------------------------------
class EmbeddedResourceStream : public IStream, public ID3DBlob
{
public:
    EmbeddedResourceStream(const char* ptr, std::size_t size)
        : mBegin(ptr), mCurrent(ptr), mEnd(ptr + size)
    {}

    ~EmbeddedResourceStream()
    {
        // From MSDN:
        //
        // The pointer returned by LockResource is valid until the module containing
        // the resource is unloaded. It is not necessary to unlock resources because
        // the system automatically deletes them when the process that created them terminates.
    }

    EmbeddedResourceStream(WORD resourceID)
    {
        const HINSTANCE hInstance = nullptr;
        
        auto hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), L"BINARY");
        Ten18_ASSERT(hResource != nullptr);
        
        auto hFileResource = LoadResource(hInstance, hResource);
        Ten18_ASSERT(hFileResource != nullptr);

        mBegin = static_cast<char*>(LockResource(hFileResource));
        Ten18_ASSERT(mBegin != nullptr);

        auto dwSize = SizeofResource(hInstance, hResource);
        Ten18_ASSERT(dwSize > 0);

        mEnd = mBegin + dwSize;
        mCurrent = mBegin;
    }
    
    Ten18_QUERY_INTERFACE_BEGIN(IStream)
        Ten18_QUERY_INTERFACE_IMPL(IStream)
        Ten18_QUERY_INTERFACE_IMPL(ISequentialStream)
        Ten18_QUERY_INTERFACE_IMPL(ID3DBlob)
    Ten18_QUERY_INTERFACE_END()

    virtual LPVOID STDMETHODCALLTYPE GetBufferPointer()
    {
        return const_cast<char*>(mBegin);
    }
    
    virtual SIZE_T STDMETHODCALLTYPE GetBufferSize()
    {
        return mEnd - mBegin;
    }
    
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
        auto n = static_cast<ULONG>(mEnd - mCurrent);
        auto ret = S_OK;
        if (cb > n)
        {
            ret = S_FALSE;
            cb = n;
        }

        memcpy_s(pv, cb, mCurrent, cb);
        mCurrent += cb;
        *pcbRead = cb;
        return ret;
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
        Ten18_UNUSED(pcbWritten);
        Ten18_UNUSED(pv);
        Ten18_UNUSED(cb);
        return STG_E_ACCESSDENIED;
    }

    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) 
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)                                      
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)                                  
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin, ULARGE_INTEGER* lpNewFilePointer)
    { 
        switch(dwOrigin)
        {
        case STREAM_SEEK_SET:
            mCurrent = mBegin + liDistanceToMove.QuadPart;
            break;
        case STREAM_SEEK_CUR:
            mCurrent += liDistanceToMove.QuadPart;
            break;
        case STREAM_SEEK_END:
            mCurrent = mEnd + liDistanceToMove.QuadPart;
            break;
        default:   
            return STG_E_INVALIDFUNCTION;
            break;
        }

        if (lpNewFilePointer != nullptr)
            lpNewFilePointer->QuadPart = mCurrent - mBegin;

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
    {
        *pStatstg = tagSTATSTG();

        pStatstg->pwcsName = (grfStatFlag & STATFLAG_NONAME) ? nullptr : L"Ten18.Cli";
        pStatstg->type = STGTY_LOCKBYTES;
        pStatstg->cbSize.QuadPart = mEnd - mBegin;
        pStatstg->grfMode = STGM_READ;
        pStatstg->clsid = CLSID_NULL;
        return S_OK;
    }

private:
    
    EmbeddedResourceStream(const EmbeddedResourceStream&);
    EmbeddedResourceStream& operator = (const EmbeddedResourceStream&);
        
    const char* mBegin;
    const char* mCurrent;
    const char* mEnd;
};

}}

