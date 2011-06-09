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
    public sealed class AppDomainManagerEx : AppDomainManager, IHeart
    {
        public AppDomainManagerEx()
        {
            Debug.Assert(AppDomain.CurrentDomain.IsDefaultAppDomain());
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        void IHeart.Rendezvous()
        {
            mTask = EntryPoint.Begin(mHeart);
        }

        void IHeart.Beat()
        {
            mHeart.Beat();
        }

        void IHeart.Farewell()
        {
            mHeart.Drain();

            Util.Dispose(ref mDebugTextWriter);
        }

        private DebugTextWriter mDebugTextWriter = new DebugTextWriter();
        private Heart mHeart = new Heart();
        private Task mTask;
    }
}
