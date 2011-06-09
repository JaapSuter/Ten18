#include "Ten18/Capture/MediaFoundationCapture.h"
#include "Ten18/Capture/MediaFoundationLogger.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/COM/COMArray.h"
#include "Ten18/Expect.h"
#include "Ten18/OnExit.h"
#include "Ten18/Timer.h"
#include "Ten18/Tracer.h"
#include "Ten18/Util.h"
#include "Ten18/Graphics/Image.h"

using namespace Ten18;
using namespace Ten18::Capture;

const static auto sHardcodedSymbolicLink
    = L"\\\\?\\usb#vid_046d&pid_0990&mi_00#7&32f118e&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\\{bbefb6c7-2fc4-4139-bb8b-a58bba724083}";

static void Release(IMFActivate*& act) { Util::Release(act); }
static COM::COMArray<UINT32, IMFActivate*> sActivates(&Release);
COM::COMPtr<IMFMediaSource> sHardcodedSource;

COM::COMPtr<IMFMediaSource> TrySymbolicLink(const wchar_t* sl)
{
    COM::COMPtr<IMFMediaSource> src;
    
    COM::COMPtr<IMFAttributes> attributes;                
    Expect.HR = MFCreateAttributes(attributes.AsTypedDoubleStar(), 2);
    Expect.HR = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    Expect.HR = attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, sl);

    auto hr = MFCreateDeviceSource(attributes.Raw(), src.AsTypedDoubleStar());
    if (SUCCEEDED(hr))    
        return std::move(src);

    src.Reset();
    return src;
}

void MediaFoundationCapture::Initialize()
{
    Ten18_ASSERT(0 == sActivates.Count());

    Expect.HR = MFStartup(MF_VERSION, MFSTARTUP_LITE);

    sHardcodedSource = TrySymbolicLink(sHardcodedSymbolicLink);
    if (sHardcodedSource)
        return;
    
    COM::COMPtr<IMFAttributes> attributes;                
    Expect.HR = MFCreateAttributes(attributes.AsTypedDoubleStar(), 1);        
    Expect.HR = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    
    Expect.HR = MFEnumDeviceSources(attributes.Raw(), sActivates.FillArray(), sActivates.FillCount());

    if (0 == sActivates.Count())
        Shutdown();
}
 
void MediaFoundationCapture::Shutdown()
{
    sHardcodedSource.Reset();
    sActivates.Reset();

    Expect.HR = MFShutdown();    
}

int MediaFoundationCapture::Count()
{
    if (sHardcodedSource)
        return 1;

    return sActivates.Count();
}

MediaFoundationCapture* MediaFoundationCapture::Get(int idx)
{
    Ten18_ASSERT(idx >= 0 && idx < Count());

    if (sHardcodedSource && idx == 0)
    {
        return Ten18_NEW MediaFoundationCapture(sHardcodedSymbolicLink, std::move(sHardcodedSource));
    }
    else
    {
        auto activate = sActivates[idx];
    
        COM::COMPtr<IMFMediaSource> mediaSource;
        Expect.HR =  activate->ActivateObject(__uuidof(mediaSource.Raw()), mediaSource.AsVoidDoubleStar());

        auto symbolicLink = Util::UsingFixedBuffer<wchar_t, UINT32, 256>([&] (wchar_t* buffer, const UINT32& max, UINT32& size) -> bool
        {
            auto hr = activate->GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, buffer, max, &size);
            return SUCCEEDED(hr);
        });

        return Ten18_NEW MediaFoundationCapture(symbolicLink.c_str(), std::move(mediaSource));
    }
}

MediaFoundationCapture::MediaFoundationCapture(const wchar_t* symbolic, COM::COMPtr<IMFMediaSource>&& mediaSource)
    : mMediaSource(std::move(mediaSource)),
      mStreamIdx(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM)),
      mStopRequest(false),
      mSymbolicLink(symbolic)
{
    COM::COMPtr<IMFAttributes> attributes;
    Expect.HR = MFCreateAttributes(attributes.AsTypedDoubleStar(), 2);
    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, static_cast<IUnknown*>(this));    
    
    Expect.HR = MFCreateSourceReaderFromMediaSource(mMediaSource.Raw(), attributes.Raw(), mSourceReader.AsTypedDoubleStar());

    const int width = 1600;
    const int height = 1200;
    ChooseBestMode(width, height);
}

void MediaFoundationCapture::ChooseBestMode(int width, int height)
{
    float fpsMax = 0;
    auto hr = S_OK;
    for (DWORD streamIdx = 0; SUCCEEDED(hr); ++streamIdx)
    {
        for (DWORD mediaTypeIdx = 0; ; ++mediaTypeIdx)
        {
            COM::COMPtr<IMFMediaType> mediaType;
            hr = mSourceReader->GetNativeMediaType(streamIdx, mediaTypeIdx, mediaType.AsTypedDoubleStar());
            if (hr == MF_E_INVALIDSTREAMNUMBER)
                break;
            else if (hr == MF_E_NO_MORE_TYPES)
                break;
            else Expect.HR = hr;

            GUID majorType;
            Expect.HR = mediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);

            if (majorType != MFMediaType_Video)
            {
                Expect.HR = mSourceReader->SetStreamSelection(streamIdx, FALSE);
                continue;
            }

            GUID subType;
            Expect.HR = mediaType->GetGUID(MF_MT_SUBTYPE, &subType);
            
            BOOL isCompressedFormat = false;
            Expect.HR = mediaType->IsCompressedFormat(&isCompressedFormat);

            UINT32 w = 0;
            UINT32 h = 0;
            Expect.HR = MFGetAttributeSize(mediaType.Raw(), MF_MT_FRAME_SIZE, &w, &h);            

            UINT32 frameRateMinNumer = 0;
            UINT32 frameRateMinDenom = 0;
            UINT32 frameRateMaxNumer = 0;
            UINT32 frameRateMaxDenom = 0;
            UINT32 frameRateNumer = 0;
            UINT32 frameRateDenom = 0;
            Expect.HR = MFGetAttributeRatio(mediaType.Raw(), MF_MT_FRAME_RATE_RANGE_MAX, &frameRateMinNumer, &frameRateMinDenom);
            Expect.HR = MFGetAttributeRatio(mediaType.Raw(), MF_MT_FRAME_RATE_RANGE_MIN, &frameRateMaxNumer, &frameRateMaxDenom);
            Expect.HR = MFGetAttributeRatio(mediaType.Raw(), MF_MT_FRAME_RATE, &frameRateNumer, &frameRateDenom);

            const auto fps = static_cast<float>(frameRateNumer) / static_cast<float>(frameRateDenom);
            if (width == static_cast<int>(w) && height == static_cast<int>(h) && fps > fpsMax && subType == MFVideoFormat_RGB24)
            {
                mSize = XMFLOAT2A(static_cast<float>(width), static_cast<float>(height));
                mBytesPerPixel = 4;
                mStreamIdx = streamIdx;
                fpsMax = fps;

                Expect.HR = mSourceReader->SetCurrentMediaType(streamIdx, nullptr, mediaType.Raw());
            }
        }
    }

    Expect.HR = mSourceReader->ReadSample(mStreamIdx, 0, nullptr, nullptr, nullptr, nullptr);
}

MediaFoundationCapture::~MediaFoundationCapture()
{
    mStopRequest = true;
    mSourceReader.Reset();
    mMediaSource->Shutdown();    
}

HRESULT MediaFoundationCapture::OnReadSample(HRESULT hr, DWORD, DWORD dwStreamFlags, LONGLONG, IMFSample* sample)
{
    OnExit oe([&]
    { 
        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            return;
        if (dwStreamFlags & MF_SOURCE_READERF_ERROR)
            return;        

        if (FAILED(hr))
            return;

        if (mStopRequest)
            return;
           
        HopeFor.HR = mSourceReader->ReadSample(mStreamIdx, 0, nullptr, nullptr, nullptr, nullptr);
    });

    if (FAILED(hr))
        return S_OK;
    
    if (!sample)
        return S_OK;
               
    DWORD bufferCount = 0;
    hr = sample->GetBufferCount(&bufferCount);
    if (FAILED(hr))
        return S_OK;

    for (DWORD b = 0; b < bufferCount; ++b)
    {
        COM::COMPtr<IMFMediaBuffer> mb;
        COM::COMPtr<IMF2DBuffer> mb2d;
        hr = sample->GetBufferByIndex(b, mb.AsTypedDoubleStar());
        if (FAILED(hr))
            return S_OK;
            
        if (mb.TryQueryInto(mb2d))
        {

        }
        else
        {
            BYTE* src = nullptr;
            DWORD maxLen = 0;
            DWORD curLen = 0;
                        
            hr = mb->Lock(&src, &maxLen, &curLen);
            if (FAILED(hr))
                return S_OK;

            OnExit oe([&] { HopeFor.HR = mb->Unlock(); });

            const int srcBytesPerPixel = 3;
            const int dstBytesPerPixel = 4;

            const int width = static_cast<int>(mSize.x);
            const int height = static_cast<int>(mSize.y);
            
            if (curLen >= static_cast<DWORD>(width * height * srcBytesPerPixel))
            {
                auto img = std::unique_ptr<Image>(static_cast<Image*>(Image::New(mSize, dstBytesPerPixel)));
                auto srcRowPitch = width * srcBytesPerPixel;
                auto srcRow = src + (height - 1) * srcRowPitch;
                auto dstPix = img->DataAs<BYTE>();
                for (int row = 0; row < width; ++row)
                {
                    auto srcPix = srcRow;
                    for (int pix = 0; pix < height; ++pix)
                    {   
                        const auto r = *srcPix++;
                        const auto g = *srcPix++;
                        const auto b = *srcPix++;
                        *dstPix++ = b;
                        *dstPix++ = g;
                        *dstPix++ = r;
                        *dstPix++ = 0xFF;
                    }
                    srcRow -= srcRowPitch;
                }
                    
                mLatest = std::move(img);
            }
        }
    }

    return S_OK;
}

void MediaFoundationCapture::Tick()
{   
}

