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
    class NativeToManaged
    {        
        public static void Emit(Assembly assembly, ModuleBuilder moduleBuilder, string cppDstDir)
        {
            var types = from type in assembly.GetExportedTypes()
                        where type.IsInterface
                          && !type.GetCustomAttributes(typeof(NativeToManagedAttribute), inherit: false).IsEmpty()
                        select type;

            foreach (var type in types)
                Emit(type, moduleBuilder, cppDstDir);
        }

        private static void Emit(Type type, ModuleBuilder moduleBuilder, string cppDstDir)
        {
            var nameParts = GetFullNameAsInCSharp(type).Split('.');
            var nameSpaceNames = nameParts.Take(nameParts.Count() - 1);
            var typeName = nameParts.Last();

            if (type.IsInterface && typeName.StartsWith("I") && char.IsUpper(typeName, 1))
                typeName = typeName.Substring(1);
            else
                typeName = typeName + "Impl";

            var fullName = String.Join(".", nameSpaceNames) + "." + typeName;

            var typeAttributes = TypeAttributes.Class | TypeAttributes.Sealed | TypeAttributes.Public;
            var typeBuilder = moduleBuilder.DefineType(fullName, typeAttributes, typeof(object), new[] { type });

            var fieldAttributes = FieldAttributes.Private | FieldAttributes.InitOnly;
            var nativePtrField = typeBuilder.DefineField("mNativePtr", PrimitiveIntPtrType, fieldAttributes);

            EmitCtors(typeBuilder, nativePtrField);

            var bindingFlags = BindingFlags.DeclaredOnly | BindingFlags.Instance | BindingFlags.Public;
            var methods = type.GetMethods(bindingFlags);
            for (int idx = 0; idx < methods.Length; ++idx)
                EmitMethod(typeBuilder, methods[idx], idx, nativePtrField);

            typeBuilder.CreateType();

            NativeToManagedCppHeader.Generate(nameSpaceNames, typeBuilder, methods, cppDstDir);
        }

        private static void EmitCtors(TypeBuilder typeBuilder, FieldInfo nativePtrField)
        {
            var nativeCtor = typeBuilder.DefinePInvokeMethod("New" + typeBuilder.Name, "Ten18.exe",
              MethodAttributes.Private | MethodAttributes.Static | MethodAttributes.PinvokeImpl,
              CallingConventions.Standard, PrimitiveIntPtrType, Type.EmptyTypes, CallingConvention.StdCall, CharSet.Unicode);
            nativeCtor.SetImplementationFlags(MethodImplAttributes.PreserveSig | nativeCtor.GetMethodImplementationFlags());

            var ctorBuilder = typeBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.HasThis, new[] { typeof(IntPtr) });
            var parameterBuilder = ctorBuilder.DefineParameter(1, ParameterAttributes.HasFieldMarshal, "nativePtr");
            
            var il = ctorBuilder.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, typeBuilder.BaseType.GetConstructor(Type.EmptyTypes));
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldarga_S, 1);
            il.Emit(OpCodes.Call, typeof(IntPtr).GetMethod("ToPointer"));
            il.Emit(OpCodes.Stfld, nativePtrField);
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
            il.Emit(OpCodes.Conv_I);
            il.Emit(OpCodes.Ldind_I);
            il.Emit(OpCodes.Ldc_I4, idx * PrimitiveIntPtrSize);
            il.Emit(OpCodes.Add);
            il.Emit(OpCodes.Conv_I);
            il.Emit(OpCodes.Ldind_I);

            il.EmitCalli(OpCodes.Calli, CallingConvention.StdCall, methodInfo.ReturnType, paramTypes.StartWith(typeof(IntPtr)).ToArray());
            il.Emit(OpCodes.Ret);
        }

        private static string GetFullNameAsInCSharp(Type type)
        {
            return mCodeProvider.GetTypeOutput(new CodeTypeReference(type));
        }

        private static CodeDomProvider mCodeProvider = CSharpCodeProvider.CreateProvider("C#");
        private static Type PrimitiveIntPtrType = typeof(Int32).MakePointerType();
        private static int PrimitiveIntPtrSize = sizeof(Int32);
    }
}
