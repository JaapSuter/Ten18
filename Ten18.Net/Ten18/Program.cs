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
    public sealed class Program : System.AppDomainManager, IManagedServiceProvider
    {
        public Program()
            : base()
        {
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        public void Rendevouz(INativeServiceProvider nativeServiceProvider)
        {
            mNativeServiceProvider = nativeServiceProvider;
            mRenderer = nativeServiceProviderpublicRenderer;
        }

        . void Tick()
        {
            throw new NotImplementedException();
        }

        private INativeServiceProvider mNativeServiceProvider;
        private IRenderer mRenderer;
    }
}
