using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;

namespace Ten18
{
    [Interop.NativeToManaged]
    public interface INativeServiceProvider
    {
        IRenderer Renderer { get; }        
    }
}
