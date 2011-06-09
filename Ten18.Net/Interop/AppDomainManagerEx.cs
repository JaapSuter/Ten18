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
        public AppDomainManagerEx()
        {
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        void IAppDomainManagerEx.Rendezvous() { mProgram = new Program(); }
        void IAppDomainManagerEx.Tick() { mProgram.Tick(); }
        void IAppDomainManagerEx.Farewell()
        {
            mProgram.Drain();
            Util.Dispose(ref mProgram);
        }

        private Program mProgram;
    }
}
