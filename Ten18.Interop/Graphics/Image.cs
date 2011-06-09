using SlimMath;
using System;

namespace Ten18.Graphics
{
    public abstract class Image : IDisposable
    {
        public Image(Vector2 size, int bpp) { }
        public abstract Vector2 Size { get; }
        public abstract void Dispose();
    }
}
