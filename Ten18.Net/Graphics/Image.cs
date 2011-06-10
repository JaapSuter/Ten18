using SlimMath;
using System;

namespace Ten18.Graphics
{
    public class Image : IDisposable
    {
        public Image(Vector2 size, int bpp) { }
        
        public extern Vector2 Size { [Native] get; }
        
        [Native]
        public extern void Dispose();
    }
}
