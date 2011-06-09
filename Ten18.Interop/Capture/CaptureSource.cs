using Ten18.Graphics;
using System;
using SlimMath;

namespace Ten18.Capture
{
    public abstract class CaptureSource : IDisposable
    {
        public CaptureSource()
        {
            WhenFrame = null;
        }

        public abstract void Dispose();

        public abstract Vector2 Size { get; }

        public IObservable<Image> WhenFrame { get; private set; }
    }
}
