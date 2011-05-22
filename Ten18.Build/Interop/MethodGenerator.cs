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
        public int VTableSlotIndex { get; private set; }

        public string NativeReturnType { get { return InteropType.Get(DoesNotFitInRegister(MethodInfo.ReturnType) ? typeof(void) : MethodInfo.ReturnType).FullNameInCpp; } }

        public string NativeParameterListOf()
        {
            return String.Join(", ",
                from pi in MethodInfo.GetParameters() select String.Format("{0}* {1}", InteropType.Get(pi.ParameterType).FullNameInCpp, pi.Name));
        }

        public MethodGenerator(InteropType interopType, MethodInfo methodInfo, int vTableSlotIndex)
        {
            Debug.Assert(methodInfo.IsPublic);
            Debug.Assert(methodInfo.IsVirtual);
            Debug.Assert(methodInfo.IsAbstract);
            Debug.Assert(!methodInfo.IsStatic); // Virtual methods can't be static anyway, but what the heck...

            MethodInfo = methodInfo;
            VTableSlotIndex = vTableSlotIndex;
        }

        public void GenerateCpp()
        {
            
        }

        public void GenerateCli(TypeBuilder typeBuilder, FieldInfo cppThisPtr)
        {
            var paramInfos = MethodInfo.GetParameters();
            var returnType = MethodInfo.ReturnType;
            var paramTypes = paramInfos.Select(pi => pi.ParameterType).ToArray();

            var returnLocal = (LocalBuilder)null;

            var methodAttributes = MethodInfo.Attributes & ~(MethodAttributes.Abstract | MethodAttributes.NewSlot) | MethodAttributes.Final;
            var methodBuilder = MethodBuilder = typeBuilder.DefineMethod(MethodInfo.Name, methodAttributes, CallingConventions.HasThis, MethodInfo.ReturnType, paramTypes);

            var il = methodBuilder.GetILGenerator();

            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, cppThisPtr);

            if (DoesNotFitInRegister(returnType))
            {
                returnLocal = il.DeclareLocal(returnType);
                il.Emit(OpCodes.Ldloca_S, returnLocal.LocalIndex);
            }

            for (int j = 0; j < paramTypes.Length; ++j)
            {
                if (Marshal.SizeOf(paramTypes[j]) > SizeOfRegisterReturn)
                    il.Emit(OpCodes.Ldarga_S, (byte)(j + 1));
                else if (j == 0) il.Emit(OpCodes.Ldarg_1);
                else if (j == 1) il.Emit(OpCodes.Ldarg_2);
                else if (j == 2) il.Emit(OpCodes.Ldarg_3);
                else il.Emit(OpCodes.Ldarg_S, (byte)(j + 1));
            }

            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Ldfld, cppThisPtr);
            il.Emit(OpCodes.Ldind_U4);
            il.Emit(OpCodes.Ldc_I4, VTableSlotIndex * VTableSlotSize);
            il.Emit(OpCodes.Add);
            il.Emit(OpCodes.Ldind_U4);

            paramTypes = DoesNotFitInRegister(returnType)
                       ? paramTypes.StartWith(typeof(IntPtr), returnType.MakeByRefType()).ToArray()
                       : paramTypes.StartWith(typeof(IntPtr)).Select(paramType => DoesNotFitInRegister(paramType)
                                                                                ? paramType.MakeByRefType()
                                                                                : paramType).ToArray();

            il.EmitCalli(OpCodes.Calli, CallingConvention.ThisCall, DoesNotFitInRegister(returnType) ? typeof(void) : MethodInfo.ReturnType, paramTypes);

            if (DoesNotFitInRegister(returnType))
                il.Emit(OpCodes.Ldloc_S, returnLocal.LocalIndex);

            il.Emit(OpCodes.Ret);

            Console.WriteLine("sizeof({0}.{1}): {2}", methodBuilder.DeclaringType.Name, methodBuilder.Name, il.ILOffset);
        }

        private static bool DoesNotFitInRegister(Type type) { return Marshal.SizeOf(type) > SizeOfRegisterReturn; }

        // Would've liked to use IntPtr, but its IL behaviour annoyed me a bit because it's a compound struct, so 
        // I dropped down to a plain old 32 bit integer, which 'll work on x86 just fine. It's not like I'll ever
        // get to a x64 or ARM port anyway... ha!
        public static readonly Type CppThisPtrType = typeof(UInt32);
        public static readonly int VTableSlotSize = sizeof(UInt32);
        public static readonly int SizeOfRegisterReturn = sizeof(UInt32);
    }
}
