#ifndef PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_SOURCE_READER_CALLBACK_HPP
#define PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_SOURCE_READER_CALLBACK_HPP

#include "Pentacorn/Capture/MediaFoundation/MediaTypeHandler.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

    class SourceReaderCallback : public IMFSourceReaderCallback
    {
    public:

        SourceReaderCallback();

        void Initialize(IMFSourceReader* sourceReader) { this->sourceReader = sourceReader; };

        STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();

        STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);

        STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) { return S_OK; }
        STDMETHODIMP OnFlush(DWORD) { return S_OK; }

        MediaTypeHandler& GetMediaTypeHandler() { return mediaTypeHandler; }

    private:
        SourceReaderCallback(const SourceReaderCallback&);
        SourceReaderCallback& operator = (const SourceReaderCallback&);
        virtual ~SourceReaderCallback();

    private:
        ULONG refCount;
        MediaTypeHandler mediaTypeHandler;
        IMFSourceReader* sourceReader;
    };

}}}

#endif
