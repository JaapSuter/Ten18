#pragma once

#include "Ten18/Capture/CaptureSource.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/ImplUnknown.h"

namespace Ten18 { namespace Capture {

    class MediaFoundationCapture : public CaptureSource, public IMFSourceReaderCallback
    {
    public:
        static void Initialize();
        static void Shutdown();

        static int Count();
        static Ptr Get(int idx);

        explicit MediaFoundationCapture(const wchar_t* symbolic, COM::COMPtr<IMFMediaSource>&& mediaSource);
        virtual ~MediaFoundationCapture();

        Ten18_QUERY_INTERFACE_BEGIN(IMFSourceReaderCallback)
            Ten18_QUERY_INTERFACE_IMPL(IMFSourceReaderCallback)
        Ten18_QUERY_INTERFACE_END()

        STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);
        STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) { return S_OK; }
        STDMETHODIMP OnFlush(DWORD) { return S_OK; }
        
    private:

        virtual void Tick();

        void ChooseBestMode(int width, int height);

        void VideoFromDiskSandboxTick();
        void VideoFromDiskSandboxInitialize();
        
        MediaFoundationCapture(const MediaFoundationCapture&);
        MediaFoundationCapture& operator = (const MediaFoundationCapture&);

        COM::COMPtr<IMFSourceReader> mSourceReader;
        COM::COMPtr<IMFMediaSource> mMediaSource;
        std::wstring mSymbolicLink;
        DWORD mStreamIdx;
        volatile bool mStopRequest;
    };

}}
