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
using SlimMath;


namespace Ten18.Interop
{
    class ClassGenerator : TypeGenerator
    {
        public ClassGenerator(Type type)
            : base(type)
        {
            Debug.Assert(type.IsClass);
            Debug.Assert(type.IsAbstract);
            Debug.Assert(!type.IsSealed);

            mMethodGenerators = type.GetMethods(BindingFlags)
                                    .Where(mi => mi.IsAbstract)
                                    .OrderBy(mi => mi.Name)
                                    .Select((mi, idx) => new MethodGenerator(mi, idx))
                                    .ToArray();
        }

        protected override void GenerateCpp(string cppRootDir)
        {
            
        }

        protected override void GenerateCli(ModuleBuilder moduleBuilder)
        {
            var fullName = String.Concat(mInteropType.FullNameInCSharp, "Impl");

            var typeAttributes = TypeAttributes.Class | TypeAttributes.Sealed | TypeAttributes.Public;
            var typeBuilder = moduleBuilder.DefineType(fullName, typeAttributes, mInteropType.Type, Type.EmptyTypes);

            var fieldAttributes = FieldAttributes.Private | FieldAttributes.InitOnly;
            var cppThisPtr = typeBuilder.DefineField("mCppThisPtr", CppThisPtrType, fieldAttributes);

            GenerateCliCtor(typeBuilder, cppThisPtr);

            mMethodGenerators.Run(mg => GenerateCliMethod(mg, typeBuilder, cppThisPtr));

            GeneratorCliProperties(typeBuilder);

            typeBuilder.CreateType();
        }


        private static void GenerateCliCtor(TypeBuilder typeBuilder, FieldInfo cppThisPtr)
        {
            var nativeCtor = typeBuilder.DefinePInvokeMethod("New" + typeBuilder.Name, "Ten18.exe",
              MethodAttributes.Private | MethodAttributes.Static | MethodAttributes.PinvokeImpl,
              CallingConventions.Standard, CppThisPtrType, Type.EmptyTypes, CallingConvention.StdCall, CharSet.Unicode);
            nativeCtor.SetImplementationFlags(MethodImplAttributes.PreserveSig | nativeCtor.GetMethodImplementationFlags());

            var ctorBuilder = typeBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.HasThis, Type.EmptyTypes);
            
            var il = ctorBuilder.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, GetFirstDefaultCtorInHierarchy(typeBuilder.BaseType));
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, nativeCtor);
            il.Emit(OpCodes.Stfld, cppThisPtr);

            il.EmitWriteLine("SizeOf in Bytecode: ");
            var localSizeOf = il.DeclareLocal(typeof(int));
            il.Emit(OpCodes.Sizeof, typeof(Vector3));
            il.Emit(OpCodes.Stloc, localSizeOf.LocalIndex);
            il.EmitWriteLine(localSizeOf);
            
            il.Emit(OpCodes.Ret);
        }

        private void GeneratorCliProperties(TypeBuilder typeBuilder)
        {
            var pgs = from propertyInfo in mInteropType.Type.GetProperties(BindingFlags)
                      let getMethodBase = propertyInfo.GetGetMethod(nonPublic: true)
                      let setMethodBase = propertyInfo.GetSetMethod(nonPublic: true)
                      where getMethodBase != null && getMethodBase.IsAbstract
                         || setMethodBase != null && setMethodBase.IsAbstract
                      let getMethodImpl = mMethodGenerators.SingleOrDefault(mg => mg.MethodInfo == getMethodBase)
                      let setMethodImpl = mMethodGenerators.SingleOrDefault(mg => mg.MethodInfo == setMethodBase)
                      where getMethodImpl != null 
                         || setMethodImpl != null
                      select new
                      {
                          PropertyInfo = propertyInfo,
                          SetMethod = setMethodImpl,
                          GetMethod = getMethodImpl,
                      };

            foreach (var pg in pgs)
            {
                var propertyBuilder = typeBuilder.DefineProperty(pg.PropertyInfo.Name, pg.PropertyInfo.Attributes, pg.PropertyInfo.PropertyType, Type.EmptyTypes);

                if (pg.GetMethod != null)
                    propertyBuilder.SetGetMethod(pg.GetMethod.MethodBuilder);

                if (pg.SetMethod != null)
                    propertyBuilder.SetSetMethod(pg.SetMethod.MethodBuilder);
            }
        }

        private static void GenerateCliMethod(MethodGenerator methodGenerator, TypeBuilder typeBuilder, FieldInfo cppThisPtr)
        {
            var methodInfo = methodGenerator.MethodInfo;
            var paramInfos = methodInfo.GetParameters();
            var paramTypes = paramInfos.Select(pi => pi.ParameterType).ToArray();

            var methodAttributes = methodInfo.Attributes & ~(MethodAttributes.Abstract | MethodAttributes.NewSlot) | MethodAttributes.Final;
            var methodBuilder = typeBuilder.DefineMethod(methodInfo.Name, methodAttributes, CallingConventions.HasThis, methodInfo.ReturnType, paramTypes);

            var il = methodBuilder.GetILGenerator();

            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, cppThisPtr);

            for (int j = 0; j < paramTypes.Length; ++j)
            {
                if      (j == 0) il.Emit(OpCodes.Ldarg_1);
                else if (j == 1) il.Emit(OpCodes.Ldarg_2);
                else if (j == 2) il.Emit(OpCodes.Ldarg_3);
                else il.Emit(OpCodes.Ldarg_S, (byte)(j + 1));
            }

            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, cppThisPtr);
            il.Emit(OpCodes.Conv_I);
            il.Emit(OpCodes.Ldind_I);
            il.Emit(OpCodes.Ldc_I4, methodGenerator.VTableSlotIndex * VTableSlotSize);
            il.Emit(OpCodes.Add);
            il.Emit(OpCodes.Conv_I);
            il.Emit(OpCodes.Ldind_I);

            if (methodInfo.Name.Contains("ThisCall"))
                il.EmitCalli(OpCodes.Calli, CallingConvention.ThisCall, methodInfo.ReturnType, paramTypes.StartWith(typeof(IntPtr)).ToArray());
            else
            il.EmitCalli(OpCodes.Calli, CallingConvention.StdCall, methodInfo.ReturnType, paramTypes.StartWith(typeof(IntPtr)).ToArray());
            il.Emit(OpCodes.Ret);

            methodGenerator.MethodBuilder = methodBuilder;
        }

        private static ConstructorInfo GetFirstDefaultCtorInHierarchy(Type type)
        {
            return type.GetConstructor(Type.EmptyTypes) ?? GetFirstDefaultCtorInHierarchy(type.BaseType);
        }

        private readonly MethodGenerator[] mMethodGenerators;

        // Would've liked to use IntPtr, but its IL behaviour annoyed me a bit because it's a compound struct, so 
        // I dropped down to a plain old 32 bit integer, which 'll work on x86 just fine. It's not like I'll ever
        // get to a x64 or ARM port anyway... ha!
        private static readonly Type CppThisPtrType = typeof(Int32).MakePointerType();
        private static readonly int VTableSlotSize = sizeof(Int32);

        private static readonly BindingFlags BindingFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;
    }
}
