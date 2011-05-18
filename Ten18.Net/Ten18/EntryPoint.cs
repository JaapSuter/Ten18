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
        public static void HostedMain(string arg)
        {
            Console.WriteLine("EntryPoint.HostedMain enter...");

            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());

            var window = new WindowImpl();

            int i0 = window.Z_StdCallFoo(1, 2, 3);
            int i1 = window.Z_ThisCallFoo(0xF1, 0xF2, 0xF3);
            
            Console.WriteLine("{0}, {1}", i0, i1);

            var position = window.Position;
            var size = window.Size;
            
            Console.WriteLine("EntryPoint.HostedMain says: {0}, ({1}, {2})", arg, position, size);

            window.MakeFullScreen();
            
            CoroutineTest.Test();
        }
    }
}
