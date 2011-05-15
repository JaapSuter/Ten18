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
    class ManagedToNative
    {
        public static void Emit(Assembly assembly, ModuleBuilder moduleBuilder, string cppDstDir)
        {
            var types = from type in assembly.GetExportedTypes()
                        where type.IsInterface
                          && !type.GetCustomAttributes(typeof(ManagedToNativeAttribute), inherit: false).IsEmpty()
                        select type;

            foreach (var type in types)
                Emit(type, moduleBuilder, cppDstDir);
        }

        private static void Emit(Type type, ModuleBuilder moduleBuilder, string cppDstDir)
        {
            var nameParts = TypeMap.GetFullNameAsInCSharp(type).Split('.');
            var nameSpaceNames = nameParts.Take(nameParts.Count() - 1);
            var typeName = nameParts.Last();

            var bindingFlags = BindingFlags.DeclaredOnly | BindingFlags.Instance | BindingFlags.Public;
            var methods = type.GetMethods(bindingFlags);

            ManagedToNativeCppHeader.Generate(nameSpaceNames, type, methods, cppDstDir);
        }

        private static void EmitCtor(TypeBuilder typeBuilder, FieldInfo managedRefField)
        {
            var ctorBuilder = typeBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.HasThis, new[] { typeof(IntPtr) });
            var il = ctorBuilder.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, typeof(object).GetConstructor(Type.EmptyTypes));
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldarg_1);
            il.Emit(OpCodes.Stfld, managedRefField);
            il.Emit(OpCodes.Ret);
        }

        private static void EmitMethod(TypeBuilder typeBuilder, MethodInfo methodInfo, int idx, FieldInfo nativePtrField)
        {
            var paramInfos = methodInfo.GetParameters();
            var paramTypes = paramInfos.Select(pi => pi.ParameterType).ToArray();

            var methodAttributes = MethodAttributes.Public | MethodAttributes.Virtual | MethodAttributes.Final | MethodAttributes.HideBySig;
            var methodBuilder = typeBuilder.DefineMethod(methodInfo.Name, methodAttributes, CallingConventions.HasThis, methodInfo.ReturnType, paramTypes);

            var il = methodBuilder.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, nativePtrField);

            for (int j = 0; j < paramTypes.Length; ++j)
            {
                if      (j == 0) il.Emit(OpCodes.Ldarg_1);
                else if (j == 1) il.Emit(OpCodes.Ldarg_2);
                else if (j == 2) il.Emit(OpCodes.Ldarg_3);
                else             il.Emit(OpCodes.Ldarg_S, (byte)(j + 1));
            }

            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, nativePtrField);
            il.Emit(OpCodes.Ldind_I);
            il.Emit(OpCodes.Ldc_I4, idx * IntPtr.Size);
            il.Emit(OpCodes.Add);
            il.Emit(OpCodes.Ldind_I);

            il.EmitCalli(OpCodes.Calli, CallingConvention.StdCall, methodInfo.ReturnType, paramTypes.StartWith(typeof(IntPtr)).ToArray());
            il.Emit(OpCodes.Ret);
        }

    }
}
