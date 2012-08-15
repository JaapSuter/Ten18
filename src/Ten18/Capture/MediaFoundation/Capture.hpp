
namespace Pentacorn { namespace Capture { namespace MediaFoundation {

	class MediaTypeHandler;

    class Capture
	{
		
	public:
        Capture();

        bool Grab(std::intptr_t section);

        int Width() const { return width; }
		int Height() const { return height; } 

	private:

        MediaTypeHandler* mediaTypeHandler;
        IMFSourceReader* sourceReader;
        int width;
		int height;

    private:

		void LogDevice(IMFActivate* activate);
        void LogMediaTypeHandler(IMFMediaTypeHandler* mediaTypeHandler);
        void SetDevice(IMFActivate* activate);
		bool MediaTypeIsAwesome(IMFMediaType *pType, IMFSourceReader* reader);
        void Enumerate();
    };

}}}
