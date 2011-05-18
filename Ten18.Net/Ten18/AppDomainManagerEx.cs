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

namespace Ten18
{    
    [ComVisible(true)]
    public sealed class AppDomainManagerEx : AppDomainManager, IAppDomainManagerEx
    {
        public AppDomainManagerEx()
            : base()
        {
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        public void Rendezvous()
        {
            EntryPoint.HostedMain("EntryPoint using AppDomainManagerEx.Rendezvous");
        }

        public void Tick()
        {
        }
    }
}
