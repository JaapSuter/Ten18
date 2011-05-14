using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;

namespace Ten18
{
    class EntryPoint
    {
        public static int HostedMain(string arg)
        {
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            Console.WriteLine("EntryPoint.HostedMain says: {0}", arg);

            Sandbox.Run();
            CoroutineTest.Test();

            return 0;
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate int InteropMainDelegate(IntPtr arg);

        public static InteropMainDelegate InteropMainDelegateClosed = new InteropMainDelegate(EntryPoint.InteropMain);

        public static int InteropMain(IntPtr arg)
        {
            return HostedMain(Marshal.PtrToStringUni(arg));
        }
    }
}
