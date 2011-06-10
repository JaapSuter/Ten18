using System;
using SlimMath;
using Ten18.Graphics;

namespace Ten18.Capture
{
    public class CaptureSource : IDisposable
    {
        public CaptureSource()
        {
            WhenFrame = null;
        }

        [Native]
        public extern void Dispose();

        public extern Vector2 Size { [Native] get; }

        public IObservable<Image> WhenFrame { get; private set; }
    }
}
