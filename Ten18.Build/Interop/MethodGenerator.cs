using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.Reflection;
using System.CodeDom;
using System.CodeDom.Compiler;
using Microsoft.CSharp;
using Ten18.Interop;
using System.IO;
using System.Linq.Expressions;
using Mono.Cecil;
using Mono.Cecil.Rocks;
using Mono.Cecil.Cil;

namespace Ten18.Interop
{
    class MethodGenerator
    {
        public MethodDefinition MethodDefinition { get; set; }        
        public int VTableSlotIndex { get; private set; }

        private static int SizeOfWhenPassedAround(TypeReference typeRef)
        {
            if (typeRef.IsValueType)
            {
                var assembly = Assembly.Load(typeRef.Resolve().Module.Assembly.Name.FullName);
                return Marshal.SizeOf(assembly.GetType(typeRef.FullName));
            }
            else return InteropType.SizeOfRegisterReturn;
        }

        public TypeReference NativeReturnTypeRef { get { return DoesNotFitInRegister(MethodDefinition.ReturnType) ? InteropType.VoidTypeRef : MethodDefinition.ReturnType; } }
        private static bool DoesNotFitInRegister(TypeReference td) { return SizeOfWhenPassedAround(td) > SizeOfRegisterReturn; }
        
        public string NativeParameterListOf()
        {
            Debug.Assert(mNativeCallSite != null);

            return String.Join(", ", 
                mNativeCallSite.Parameters.Skip(1).Select(pi =>
                {
                    if (pi.ParameterType.IsByReference)
                        if (pi.IsOut)
                            return InteropType.Get(pi.ParameterType).FullNameInCpp + "& " + pi.Name;
                        else
                            return "const " + InteropType.Get(pi.ParameterType).FullNameInCpp + "& " + pi.Name;
                    else
                        if (pi.ParameterType.IsPrimitive)
                            return InteropType.Get(pi.ParameterType).FullNameInCpp + " " + pi.Name;
                        else
                            return InteropType.Get(pi.ParameterType).FullNameInCpp + "* " + pi.Name;
                }));
        }

        public MethodGenerator(MethodDefinition methodDefinition, int vTableSlotIndex)
        {
            MethodDefinition = methodDefinition;
            VTableSlotIndex = vTableSlotIndex;
            
            Debug.Assert(methodDefinition.IsAbstract);
            Debug.Assert(!methodDefinition.IsStatic);
        }

        public void GenerateCli(FieldDefinition cppThisPtrDef)
        {
            MethodDefinition.IsAbstract = false;
            MethodDefinition.IsVirtual = true;
            MethodDefinition.IsNewSlot = false;
            MethodDefinition.IsHideBySig = true;

            var paramDefs = MethodDefinition.Parameters;
            var returnTypeRef = MethodDefinition.ReturnType;
            var returnTypeIsLarge = DoesNotFitInRegister(returnTypeRef.Resolve());
            var nativeReturnTypeDef = returnTypeIsLarge ? MethodDefinition.Module.TypeSystem.Void : returnTypeRef;
            var ilp = MethodDefinition.Body.GetILProcessor();

            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldfld, cppThisPtrDef);

            VariableDefinition returnLocal = null;
            if (returnTypeIsLarge)
            {
                returnLocal = new VariableDefinition("ret", returnTypeRef);
                MethodDefinition.Body.Variables.Add(returnLocal);
                MethodDefinition.Body.InitLocals = true;

                ilp.Emit(OpCodes.Ldloca_S, (byte)returnLocal.Index);
            }

            for (int j = 0; j < paramDefs.Count; ++j)
            {
                if (SizeOfWhenPassedAround(paramDefs[j].ParameterType.Resolve()) > SizeOfRegisterReturn)
                    ilp.Emit(OpCodes.Ldarga_S, (byte)(j + 1));
                else if (j == 0) ilp.Emit(OpCodes.Ldarg_1);
                else if (j == 1) ilp.Emit(OpCodes.Ldarg_2);
                else if (j == 2) ilp.Emit(OpCodes.Ldarg_3);
                else ilp.Emit(OpCodes.Ldarg_S, (byte)(j + 1));
            }

            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldfld, cppThisPtrDef);
            ilp.Emit(OpCodes.Ldind_I4);
            ilp.Emit(OpCodes.Ldc_I4, VTableSlotIndex * InteropType.VTableSlotSize);
            ilp.Emit(OpCodes.Add);
            ilp.Emit(OpCodes.Ldind_I4);

            Debug.Assert(mNativeCallSite == null);
            mNativeCallSite = new CallSite(nativeReturnTypeDef)
            {
                CallingConvention = MethodCallingConvention.ThisCall,
            };

            mNativeCallSite.Parameters.Add(new ParameterDefinition("cppThisPtr", Mono.Cecil.ParameterAttributes.In, InteropType.CppThisPtrTypeRef));
            if (returnTypeIsLarge)
                mNativeCallSite.Parameters.Add(new ParameterDefinition("ret", Mono.Cecil.ParameterAttributes.Out, returnTypeRef.MakeByReferenceType()));
            foreach (var parameterDef in MethodDefinition.Parameters)
            {
                var nativeParameterDef = DoesNotFitInRegister(parameterDef.ParameterType.Resolve())
                                       ? new ParameterDefinition(parameterDef.Name, parameterDef.Attributes, parameterDef.ParameterType.MakeByReferenceType())
                                       : new ParameterDefinition(parameterDef.Name, parameterDef.Attributes, parameterDef.ParameterType);                
                mNativeCallSite.Parameters.Add(nativeParameterDef);
            }

            ilp.Emit(OpCodes.Calli, mNativeCallSite);

            if (returnTypeIsLarge)
                ilp.Emit(OpCodes.Ldloc_S, (byte)returnLocal.Index);

            ilp.Emit(OpCodes.Ret);
        }

        private CallSite mNativeCallSite;
        
        public static readonly int SizeOfRegisterReturn = sizeof(Int32);
    }
}
