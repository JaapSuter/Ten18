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
    [GenerateCppDefinition]
    public struct Vector3
    {
        public float X, Y, Z;
    }

    [GenerateCppDefinition]
    public abstract class Window
    {
        // public abstract Vector2 Size { get; set; }
        // public abstract Vector2 Position { get; set; }
        
        public abstract float Position { get; set; }
        public abstract int Size { get; set; }
        
        public abstract bool IsFullScreen { get; }
        public abstract void MakeFullScreen();

        public abstract int Z_StdCallFoo(int a0, int a1, int a2);
        public abstract int Z_ThisCallFoo(int a0, int a1, int a2);
    }
}
