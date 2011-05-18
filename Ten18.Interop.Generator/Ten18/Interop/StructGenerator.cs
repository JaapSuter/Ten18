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
using Microsoft.CSharp;
using Ten18.Interop;
using System.IO;
using System.Linq.Expressions;


namespace Ten18.Interop
{
    class StructGenerator : TypeGenerator
    {
        public StructGenerator(Type type)
            : base(type)
        {
            Debug.Assert(type.IsValueType);
            Debug.Assert(type.IsLayoutSequential);
        }

        protected override void GenerateCpp(string cppRootDir)
        {
        }

        protected override void GenerateCli(ModuleBuilder moduleBuilder)
        {
        }
    }
}
