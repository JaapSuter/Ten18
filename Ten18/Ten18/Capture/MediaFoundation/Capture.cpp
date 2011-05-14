#include "Pentacorn/Precompiled.Cpp.hpp"
#include "Pentacorn/Capture/MediaFoundation/Capture.hpp"
#include "Pentacorn/Capture/MediaFoundation/SourceReaderCallback.hpp"
#include "Pentacorn/Capture/MediaFoundation/MediaFoundation.hpp"
#include "Pentacorn/Capture/MediaFoundation/MediaTypeHandler.hpp"
#include "Pentacorn/Capture/MediaFoundation/MediaTypeGrokker.hpp"
#include "Pentacorn/CountedPtr.hpp"
#include "Pentacorn/ComArray.hpp"
#include "Pentacorn/Error.hpp"

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

Capture::Capture()
	: mediaTypeHandler(new MediaTypeHandler())
{
    MediaFoundation::Startup();            
    Enumerate();            
}

bool Capture::Grab(std::intptr_t section)
{
    if (sourceReader == nullptr)
        return false;

    DWORD actualStreamIndex = 0;
    DWORD streamFlags = 0;
    LONGLONG timestamp = 0;
    IMFSample* pSample = nullptr;

    HRESULT hr = sourceReader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), 0, &actualStreamIndex, &streamFlags, &timestamp, &pSample);
    if (FAILED(hr))
        return false;

    if (nullptr == pSample)
        return false;
                        
    CountedPtr<IMFMediaBuffer> buffer;
    hr = pSample->GetBufferByIndex(0, buffer.Fill());
    if (FAILED(hr))
        return false;
            
    mediaTypeHandler->DrawFrame(buffer.Get());
                                    
    unsigned char* src = mediaTypeHandler->m_buffer;
    if (src == nullptr)
        return false;
            
	static_assert(sizeof(HANDLE) == sizeof(section), "Unexpected sizeof(Handle)");
    const int bpp = 4;
    void* dst = MapViewOfFile(reinterpret_cast<HANDLE>(section), FILE_MAP_WRITE, 0, 0, width * height * bpp);
    if (dst == nullptr)
        return false;

    std::memcpy(dst, src, width * height * bpp);

    return true;
}

void Capture::LogDevice(IMFActivate* activate)
{
	CountedPtr<IMFMediaSource> source;
    Error = activate->ActivateObject(__uuidof(IMFMediaSource), source.Fill());
    
	CountedPtr<IMFPresentationDescriptor> presentationDescriptor;
	Error = source->CreatePresentationDescriptor(presentationDescriptor.Fill());
				
	DWORD descriptorCount = 0;
	Error = presentationDescriptor->GetStreamDescriptorCount(&descriptorCount);
	
    for (DWORD i = 0; i < descriptorCount; ++i)
	{
        CountedPtr<IMFStreamDescriptor> streamDescriptor;
		BOOL selected = false;
		Error = presentationDescriptor->GetStreamDescriptorByIndex(i, &selected, streamDescriptor.Fill());
		
		CountedPtr<IMFMediaTypeHandler> mediaTypeHandler;
		Error = streamDescriptor->GetMediaTypeHandler(mediaTypeHandler.Fill());
		
		LogMediaTypeHandler(mediaTypeHandler.Get());
  	}

    // Todo, Jaap Suter, June 2010: calling IMFMediaSource::Shutdown should do the same, but for some reason made later
    // reactivations of the source go wonkers. This works for now, but some time I should look into why this isn't 
    // working as I expected.
    Error = activate->ShutdownObject();
}

void Capture::LogMediaTypeHandler(IMFMediaTypeHandler* mediaTypeHandler)
{
    DWORD mediaTypeCount = 0;
	Error = mediaTypeHandler->GetMediaTypeCount(&mediaTypeCount);
	
	for (DWORD i = 0; i < mediaTypeCount; ++i)
	{
        CountedPtr<IMFMediaType> mediaType;
		Error = mediaTypeHandler->GetMediaTypeByIndex(i, mediaType.Fill());
		
		MediaTypeGrokker mediaTypeGrokker(mediaType.Get());
	}
}

void Capture::SetDevice(IMFActivate* activate)
{
    CountedPtr<IMFMediaSource> source;
    Error = activate->ActivateObject(__uuidof(IMFMediaSource), source.Fill());
    
    WCHAR* pwszSymbolicLink = nullptr;
    std::uint32_t cchSymbolicLink = 0;
    Error = activate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &pwszSymbolicLink, &cchSymbolicLink);
    
    CountedPtr<IMFAttributes> attributes;            
    const UINT32 numAttributes = 1;
    Error = MFCreateAttributes(attributes.Fill(), numAttributes);
    
	// Todo, Jaap Suter, June 2010: 2 up and callback again.
    Error = attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, FALSE);
    
    CountedPtr<IMFSourceReader> reader;
    Error = MFCreateSourceReaderFromMediaSource(source.Get(), attributes.Get(), reader.Fill());
    
    sourceReader = reader.Get();

    for (DWORD i = 0; ; i++)
    {
        CountedPtr<IMFMediaType> mediaType;
            
        Error = reader->GetNativeMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), i, mediaType.Fill());
        
        MediaTypeGrokker a(mediaType.Get());

        Error = MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, 5, 1);
        
        MediaTypeGrokker b(mediaType.Get());

        // Todo, MFMediaType::GetRepresentation

        if (MediaTypeIsAwesome(mediaType.Get(), reader.Get()))
        {
            // Error = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, mediaType.Get());
            
            width = mediaTypeHandler->m_width;
            height = mediaTypeHandler->m_height;

            mediaType->AddRef();
            break;
        }
    }

    reader->AddRef();
    source->AddRef();
}

bool Capture::MediaTypeIsAwesome(IMFMediaType *pType, IMFSourceReader* reader)
{
    GUID subtype;
    Error = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
    
    if (mediaTypeHandler->IsFormatSupported(subtype))
    {
        mediaTypeHandler->SetVideoType(pType);
        return true;
    }
    else
    {
        for (int i = 0;  ;++i)
        {
            const GUID* subtypePtr = mediaTypeHandler->GetFormat(i);
            if (subtypePtr)
                return false;

            subtype = *subtypePtr;
            HRESULT hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);
            if (FAILED(hr))
                return false;

            hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, pType);
            if (SUCCEEDED(hr))
            {
                mediaTypeHandler->SetVideoType(pType);
                return true;
            }
        }
    }
}

void Capture::Enumerate()
{
    CountedPtr<IMFAttributes> attributes;
                
    Error = MFCreateAttributes(attributes.Fill(), 1);    
        
    Error = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        
	ComArray<UINT32, IMFActivate*> activates;

    Error = MFEnumDeviceSources(attributes.Get(), activates.FillArray(), activates.FillCount());
        
    if (activates.Count() > 0)
    {
		for (UINT32 i = 0; i < activates.Count(); ++i)
		{                        
            LogDevice(activates[i]);
		}

        SetDevice(activates[0]);
    }
}

}}}
