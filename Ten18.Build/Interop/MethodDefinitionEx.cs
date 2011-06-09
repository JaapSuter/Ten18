using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.CodeDom;
using System.CodeDom.Compiler;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using Microsoft.CSharp;
using System.IO;
using Mono.Cecil;
using Mono.Cecil.Rocks;

namespace Ten18.Interop
{
    static class MethodDefinitionEx
    {
        public static string FullNameAsCpp(this MethodDefinition self)
        {
            Debug.Assert(self.IsPInvokeImpl);
            Debug.Assert(self.ReturnType.CanBePassedAroundInRegister());

            // We can't check self.IsStatic because when it's a PInvoke implementation, it'll be always
            // static. The point being that it's not about the managed this pointer, but the native this pointer
            // we care about.
            var isStatic = self.Parameters.IsEmpty() || self.Parameters.First().Name != Globals.NameOfCppThisPtrParameter;

            var parameterDefs = isStatic ? self.Parameters : self.Parameters.Skip(1);
            var callingConv = isStatic ? "__stdcall" : "__thiscall";
            var staticOrNot = isStatic ? "static " : "";

            return String.Format("{0}{1} {2} {3}({4});",
                staticOrNot,
                self.ReturnType.FullNameAsCpp(isConst: false),
                callingConv,
                self.Name,
                String.Join(", ", parameterDefs.Select(parameterDef => parameterDef.FullNameAsCpp())));
        }

        public static string FullNameAsUndecorated(this MethodDefinition self)
        {
            // Todo, Jaap Suter, June 2011, merge with function above somehow.
            
            Debug.Assert(self.IsPInvokeImpl);
            Debug.Assert(self.ReturnType.CanBePassedAroundInRegister());

            var isStatic = self.Parameters.IsEmpty() || self.Parameters.First().Name != Globals.NameOfCppThisPtrParameter;

            var parameterDefs = isStatic ? self.Parameters : self.Parameters.Skip(1);
            var callingConv = isStatic ? "__stdcall" : "__thiscall";
            var staticOrNot = isStatic ? "static " : "";

            return String.Format("public: {0}{1} {2} {3}::{4}({5})",
                staticOrNot,
                self.ReturnType.FullNameAsUndecorated(isConst: false),
                callingConv,
                self.DeclaringType.FullName.Replace(".", "::"),
                self.Name,
                String.Join(", ", parameterDefs.Select(parameterDef => parameterDef.FullNameAsUndecorated())));
        }
    }
}
