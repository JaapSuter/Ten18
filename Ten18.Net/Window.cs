using System;
using SlimMath;

namespace Ten18
{
    public class Window : IDisposable
    {
        public Window(string title) { }

        public extern bool HasClosed { [Native] get; }

        public extern Vector2 Size { [Native] get; [Native] set; }        
        public extern Vector2 Position { [Native] get; [Native] set; }
        public extern bool IsFullScreen { [Native] get; }
        
        [Native]
        public extern void MakeFullScreen();

        [Native]
        public extern void Dispose();
    }
}
