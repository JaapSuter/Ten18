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
using Mono.Cecil;


namespace Ten18.Interop
{
    class StructGenerator : TypeGenerator
    {
        public StructGenerator(TypeDefinition typeDef)
            : base(typeDef)
        {
            Debug.Assert(typeDef.IsValueType);
            Debug.Assert(typeDef.HasLayoutInfo);
        }

        protected override void GenerateCpp()
        {
        }

        protected override void GenerateCli()
        {
        }
    }
}
