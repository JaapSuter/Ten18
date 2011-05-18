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
        private static void ThrowingMain()
        {
            var assembly = typeof(GenerateCppDefinitionAttribute).Assembly;
            var generator = new Generator(assembly);
            
            generator.Generate();
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
