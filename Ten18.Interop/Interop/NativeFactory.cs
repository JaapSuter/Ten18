using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;

namespace Ten18.Interop
{
    public abstract class NativeFactory
    {
        #pragma warning disable 649 // Field is never assigned to, and will always have its default value null
        public static NativeFactory Instance;
    }   
}
