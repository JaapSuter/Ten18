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
    [ComVisible(true)]
    [Guid("37D42543-4326-4A49-9320-A1BF1716AB54")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface IAppDomainManagerEx
    {
        [PreserveSig] void Rendezvous();
        [PreserveSig] void Tick();
        [PreserveSig] void Farewell();
    }
}
