using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.Reflection.Emit;
using System.Reflection;
using System.CodeDom;
using System.CodeDom.Compiler;
using System.Runtime.CompilerServices;
using Ten18.Interop;
using System.Linq.Expressions;
using System.IO;

namespace Ten18.Interop
{
    class EntryPoint
    {
        public static int Main(params string[] args)
        {
            if (args.Length != 1 || !File.Exists(args[0]))
            {
                Console.WriteLine("Usage: Ten18.Interop.Generator Path/To/DotNetAssembly.Dll");
                return -1;
            }
            else
            {
                Console.WriteLine("Generating From: {0}", args[0]);
                new AssemblyGenerator(Assembly.LoadFile(args[0])).Generate();
                return 0;
            }
        }
    }
}
