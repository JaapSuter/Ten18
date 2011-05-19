using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;
using Ten18.Interop;

namespace Ten18
{
    static class EntryPoint
    {
        class Foo
        {
            public virtual Vector3 LargeReturn()
            {
                return new Vector3() { X = 424242, Y = 2, Z = 666 };
            }
        }

        public static void HostedMain(string arg)
        {
            Console.WriteLine("EntryPoint.HostedMain enter...");

            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            
            var f = new Foo();
            var i0 = new Vector3();


            i0 = f.LargeReturn();
            Console.WriteLine("({0}, {1}, {2})", i0.X, i0.Y, i0.Z);
            
            var window = new WindowImpl();

            var v3 = new Vector3() { X = 1, Y = 2, Z = 3 };
            var i1 = window.Z_ThisCallFoo(0xF1, ref v3, 0xF3);

            Console.WriteLine("({0}, {1}, {2})", i1.X, i1.Y, i1.Z);

            var position = window.Position;
            var size = window.Size;
            
            Console.WriteLine("EntryPoint.HostedMain says: {0}, ({1}, {2})", arg, position, size);

            window.MakeFullScreen();
            
            CoroutineTest.Test();
        }
    }
}
