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
using System.Linq.Expressions;
using System.IO;
using Ten18.Interop;

namespace Ten18
{
    class EntryPoint
    {
        private static void ThrowingMain()
        {
            var assembly = typeof(IAppDomainManagerEx).Assembly;
            var generator = new AssemblyGenerator(assembly);
            
            generator.Generate();

            using (var tw = File.CreateText(Path.Combine(Paths.SolutionDir, @"obj\x86\Debug\Ten18.Interop.Impl.dll.h")))
                BinToHeader.Convert(File.ReadAllBytes(assembly.Location), tw, "g_Ten18_Interop_Impl_dll");
        }

        public static int Main()
        {
            if (Debugger.IsAttached)
            {
                ThrowingMain();   
            }
            else
            {
                try
                {
                    ThrowingMain();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                    return -1;
                }
            }

            return 0;
        }
    }
}
