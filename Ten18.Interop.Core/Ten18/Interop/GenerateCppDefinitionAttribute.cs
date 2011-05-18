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
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Enum | AttributeTargets.Struct, AllowMultiple = false, Inherited = true)]
    public class GenerateCppDefinitionAttribute : System.Attribute
    {
    }
}
