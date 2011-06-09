using System;
using SlimMath;

namespace Ten18
{
    public abstract class Window : IDisposable
    {
        public Window(string title) { }

        public abstract bool HasClosed { get; }

        public abstract Vector2 Size { get; set; }
        public abstract Vector2 Position { get; set; }
        
        public abstract bool IsFullScreen { get; }
        public abstract void MakeFullScreen();

        public abstract void Dispose();
    }
}
