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
        class Foo
        {
            public Foo(int i) { cppt = i; }
            public virtual void Blah()
            {
                PeeInvokeDeleteWindow(cppt);
            }
            public int cppt;
        }

        [DllImport("Ten18.exe", PreserveSig=true, SetLastError=false, CallingConvention=CallingConvention.StdCall, CharSet=CharSet.Unicode)]
        private static extern int PeeInvokeNewWindow([MarshalAs(UnmanagedType.LPWStr)] string title, int x, int y);

        [DllImport("Ten18.exe", PreserveSig=true, SetLastError=false, CallingConvention=CallingConvention.StdCall, CharSet=CharSet.Unicode)]
        private static extern void PeeInvokeDeleteWindow(int wnd);

        [DllImport("Ten18.exe", PreserveSig = true, SetLastError = false, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern void PeeInvokeSet(int wnd, [In] ref Vector2 v);
        
        [DllImport("Ten18.exe", PreserveSig = true, SetLastError = false, CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern void PeeInvokeGet(int wnd, [Out] out Vector2 v);

        public static void HostedMain(string arg)
        {
            Console.WriteLine("EntryPoint.HostedMain Enter: {0}", arg);
            
            const int n = 1000000;
            Benchmark.Run("PeeInvoke", () =>
            {
                for (int i = 0; i < n; ++i)
                {
                    var t = new Foo(PeeInvokeNewWindow("Yo dawg: " + arg, 123, 456));
                    Vector2 v;
                    PeeInvokeGet(t.cppt, out v);
                    PeeInvokeSet(t.cppt, ref v);
                    t.Blah();
                }
            });

            Benchmark.Run("OurInvoke", () =>
            {
                for (int i = 0; i < n; ++i)
                {
                    var w = new Window("Yo dawg: " + arg, 123, 456);
                    w.Position = w.Size;
                    w.Dispose();
                }
            });
            
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            
            using (var window = new Window("Hallekiedee", 1, 2))
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
