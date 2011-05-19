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
    public struct Vector3
    {
        public float X, Y, Z;
    }

    public abstract class Window
    {
        // public abstract Vector2 Size { get; set; }
        // public abstract Vector2 Position { get; set; }
        
        public abstract float Position { get; set; }
        public abstract int Size { get; set; }
        
        public abstract bool IsFullScreen { get; }
        public abstract void MakeFullScreen();

        public abstract Vector3 Z_ThisCallFoo(int a0, ref Vector3 a1, int a2);
    }
}
