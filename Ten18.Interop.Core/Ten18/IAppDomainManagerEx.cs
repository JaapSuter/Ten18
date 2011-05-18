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
    [Guid("37D42543-4326-4A49-9320-A1BF1716AB54")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppDomainManagerEx
    {
        [PreserveSig]
        void Rendezvous();

        [PreserveSig]
        void Tick();
    }
}
