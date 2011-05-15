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
    static class EntryPoint
    {
        public static int HostedMain(string arg)
        {
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            Console.WriteLine("EntryPoint.HostedMain says: {0}", arg);

            CoroutineTest.Test();

            return 0;
        }
    }
}
