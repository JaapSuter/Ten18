using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;
using SlimMath;

namespace Ten18
{
    static class EntryPoint
    {
        public static void HostedMain(string arg)
        {
            Console.WriteLine("EntryPoint.HostedMain Enter...: " + arg);
            
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            
            using (var window = new Window())
            {
                var v1 = new Vector4(3, 5, 6, 5);
                var v2 = window.Position;
                Console.WriteLine("({0}, {1})", v2.X, v2.Y);

                window.Position = new Vector2(30, 40);
                v2 = window.Position;
                Console.WriteLine("({0}, {1})", v2.X, v2.Y);

                var position = window.Position;
                var size = window.Size;
            
                Console.WriteLine("EntryPoint.HostedMain Says: {0}, ({1}, {2})", arg, position, size);

                window.MakeFullScreen();
            }
            
            // CoroutineTest.Test();
        }
    }
}
