using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Collections.Generic;
using Ten18.Interop;

namespace Ten18.Hosting
{
    [ComVisible(true)]
    [Guid("B0A493DA-A684-4AC9-B823-7EEA1CAB0A04")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [ManagedToNative]
    public interface IManagedServiceProvider
    {
        [PreserveSig]
        int TwoArgsOneReturnTest(int a, int b);        
    }
}
