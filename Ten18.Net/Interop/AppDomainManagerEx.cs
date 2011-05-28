using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;
using SlimMath;

namespace Ten18.Interop
{
    public sealed class AppDomainManagerEx : AppDomainManager, IAppDomainManagerEx
    {
        public AppDomainManagerEx() { InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost; }

        void IAppDomainManagerEx.Rendezvous(IntPtr nativeFactory) { mProgram = new Program(nativeFactory); }
        void IAppDomainManagerEx.Tick() { mProgram.Tick(); }
        void IAppDomainManagerEx.Farewell() { Util.Dispose(ref mProgram); }

        private Program mProgram;
    }
}
