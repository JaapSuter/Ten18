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

namespace Ten18.Interop
{
    // Random class that has the method signature I want
    public class MyClass
    {
        public void MyConsoleWriteLineMethod(string output)
        {
            Console.WriteLine(output);
        }
    }

    /*
        The ILDASM output on ReflectionEmitDelegateTest.dll looks like this:

        .class public auto autochar sealed MyDelegateType
               extends [mscorlib]System.MulticastDelegate
        {
          .method public hidebysig specialname rtspecialname
                  instance void  .ctor(object A_1,
                                       native int A_2) runtime managed
          {
          } // end of method MyDelegateType::.ctor
 
          .method public hidebysig newslot virtual
                  instance void  Invoke(string A_1) runtime managed
          {
          } // end of method MyDelegateType::Invoke
 
        } // end of class MyDelegateType
    */

    static class DelegateGenerator
    {
        public static Type Generate(TypeBuilder declaringTypeBuilder, MethodInfo methodInfo)
        {
            var typeBuilder = declaringTypeBuilder.DefineNestedType(methodInfo.Name + "Delegate", TypeAttributes.Class | TypeAttributes.NestedPublic | TypeAttributes.Sealed | TypeAttributes.UnicodeClass, typeof(System.MulticastDelegate));
            var ctorBuilder = typeBuilder.DefineConstructor(MethodAttributes.RTSpecialName | MethodAttributes.HideBySig | MethodAttributes.Public, CallingConventions.Standard, new Type[] { typeof(object), typeof(System.IntPtr) });            
            ctorBuilder.SetImplementationFlags(MethodImplAttributes.Runtime | MethodImplAttributes.Managed);

            var paramInfos = methodInfo.GetParameters();
            var paramTypes = paramInfos.Select(pi => pi.ParameterType).ToArray();

            var methodBuilder = typeBuilder.DefineMethod("Invoke", MethodAttributes.Public | MethodAttributes.HideBySig | MethodAttributes.NewSlot | MethodAttributes.Virtual, methodInfo.ReturnType, paramTypes);
            methodBuilder.SetImplementationFlags(MethodImplAttributes.Runtime | MethodImplAttributes.Managed);

            return typeBuilder.CreateType();
        }
    }
}
