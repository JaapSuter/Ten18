using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Linq.Expressions;
using System.Diagnostics;
using System.Disposables;

namespace Ten18.Interop
{    
    public sealed class AppDomainManagerEx : AppDomainManager, IAppDomainManagerEx
    {
        public AppDomainManagerEx() : base()
        {
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        void IAppDomainManagerEx.Rendezvous(IntPtr nativeFactory)
        {
            NativeFactory.Instance = new NativeFactory(nativeFactory);
        }

        void IAppDomainManagerEx.Tick()
        {
            EntryPoint.HostedMain("EntryPoint using AppDomainManagerEx.Tick");
        }
    }
}
