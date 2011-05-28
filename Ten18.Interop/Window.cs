using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using SlimMath;
using Ten18.Interop;
using System.Runtime.CompilerServices;

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
