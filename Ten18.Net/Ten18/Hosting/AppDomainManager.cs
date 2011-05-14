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

namespace Ten18.Hosting
{    
    [ComVisible(true)]
    public sealed class AppDomainManager : System.AppDomainManager, IManagedServiceProvider // ICustomQueryInterface, 
    {
        public AppDomainManager() : base()
        {
            // var nativeFuncPtr = Marshal.GetFunctionPointerForDelegate(EntryPoint.InteropMainDelegateClosed);
            // AnnounceManagedServiceProvider(nativeFuncPtr);
            InitializationFlags = AppDomainManagerInitializationOptions.RegisterWithHost;
        }

        /*
        public CustomQueryInterfaceResult GetInterface(ref Guid iid, out IntPtr ppv)
        {
            ppv = IntPtr.Zero;
            if (iid == new Guid("B0A493DA-A684-4AC9-B823-7EEA1CAB0A04"))
            {
                ppv = Marshal.GetIUnknownForObject(this);
                return CustomQueryInterfaceResult.Handled;
            }
            else return CustomQueryInterfaceResult.NotHandled;
        }
        */

        public int TwoArgsOneReturnTest(int a, int b)
        { 
            return a + b + mMember;
        }

        private int mMember = 45;
    }
}
