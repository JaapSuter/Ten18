#ifndef PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_TYPE_GROKKER_HPP
#define PENTACORN_NATIVE_CAPTURE_MEDIA_FOUNDATION_MEDIA_TYPE_GROKKER_HPP

namespace Pentacorn { namespace Capture { namespace MediaFoundation {

    class MediaTypeGrokker
    {
    public:
		MediaTypeGrokker(IMFMediaType* mediaType);

	private:
        MediaTypeGrokker(const MediaTypeGrokker&);
        MediaTypeGrokker& operator = (const MediaTypeGrokker&);

    private:
    };

}}}

#endif
