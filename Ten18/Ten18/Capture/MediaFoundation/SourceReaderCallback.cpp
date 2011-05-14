#include "Pentacorn/Precompiled.Cpp.hpp"
#include "Pentacorn/Capture/MediaFoundation/SourceReaderCallback.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

SourceReaderCallback::SourceReaderCallback()
    : refCount(1), sourceReader()
{}

HRESULT SourceReaderCallback::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(SourceReaderCallback, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG SourceReaderCallback::AddRef()
{
    return InterlockedIncrement(&refCount);
}

ULONG SourceReaderCallback::Release()
{
    ULONG rc = InterlockedDecrement(&refCount);
    if (rc == 0)
        delete this;    
    return rc;
}

SourceReaderCallback::~SourceReaderCallback()
{
}

// Todo, shuffle
_COM_SMARTPTR_TYPEDEF(IMFMediaBuffer, __uuidof(IMFMediaBuffer));
#pragma warning(disable: 4100)

HRESULT SourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)
{
    if (FAILED(hrStatus))
        return hrStatus;
    
    if (nullptr != pSample)
    {            
        IMFMediaBufferPtr buffer;
        HRESULT hr = pSample->GetBufferByIndex(0, &buffer);
        if (SUCCEEDED(hr))
        {
            mediaTypeHandler.DrawFrame(buffer);
        }
    }
    
    return sourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), 0, nullptr, nullptr, nullptr, nullptr);
}
    

}}}
