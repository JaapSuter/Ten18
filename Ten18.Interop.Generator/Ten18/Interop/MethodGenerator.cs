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
    class MethodGenerator
    {
        public MethodBuilder MethodBuilder { get; set; }
        
        public MethodInfo MethodInfo { get; private set; }        
        public string CliSignature { get; private set; }
        public string CppSignature { get; private set; }
        public int VTableSlotIndex { get; private set; }

        public MethodGenerator(MethodInfo methodInfo, int vTableSlotIndex)
        {
            Debug.Assert(methodInfo.IsPublic);
            Debug.Assert(methodInfo.IsVirtual);
            Debug.Assert(methodInfo.IsAbstract);
            Debug.Assert(!methodInfo.IsStatic); // Virtual methods can't be static anyway, but what the heck...

            MethodInfo = methodInfo;
            VTableSlotIndex = vTableSlotIndex;

            CliSignature = String.Format("{0} {1}({2})",
                InteropType.Get(methodInfo.ReturnType).FullNameInCSharp,
                methodInfo.Name,
                String.Join(",", methodInfo.GetParameters().Select(pg => String.Format("{0} {1}", InteropType.Get(pg.ParameterType).FullNameInCSharp, pg.Name))));

            CppSignature = String.Format("{0} {1}({2})",
                InteropType.Get(methodInfo.ReturnType).FullNameInCpp,
                methodInfo.Name,
                String.Join(",", methodInfo.GetParameters().Select(pg => String.Format("{0}{1}{2}",
                    InteropType.Get(pg.ParameterType).FullNameInCpp,
                    pg.ParameterType.IsInterface ? "* " : " ",
                    pg.Name))));
        }
    }
}
