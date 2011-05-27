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
using Microsoft.CSharp;
using Ten18.Interop;
using System.IO;
using System.Linq.Expressions;
using Mono.Cecil;
using Mono.Cecil.Rocks;
using Mono.Cecil.Cil;
using Mono.Collections.Generic;

namespace Ten18.Interop
{
    static class MethodGenerator
    {
        public static void PatchConstructor(TypeDefinition typeDef, MethodDefinition ctorDef, FieldDefinition cppThisPtr)
        {
            Debug.Assert(ctorDef.IsConstructor);

            var methodDef = new MethodDefinition("New" + typeDef.Name, MethodAttributes.Abstract | MethodAttributes.Assembly, TypeRefs.VoidStar);
            ctorDef.Parameters.Run(p => methodDef.Parameters.Add(new ParameterDefinition(p.Name, p.Attributes, p.ParameterType)));

            TypeRefs.NativeFactory.Methods.Add(methodDef);

            var ilp = ctorDef.Body.GetILProcessor();
            var offset = ctorDef.Body.Instructions.First(i => i.OpCode == OpCodes.Call).Next;

            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarg_0));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldsfld, TypeRefs.NativeFactory.Fields.First(fd => fd.Name == "Instance")));
            
            foreach(var parameterDef in methodDef.Parameters)
                ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarg, parameterDef));

            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Call, methodDef));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Stfld, cppThisPtr));

            ctorDef.Body.OptimizeMacros();
        }

        public static void Generate(TypeDefinition typeDef, MethodDefinition methodDef, FieldDefinition cppThisPtr, int vTableSlotIdx, CppHeaderTemplate cppHeaderTemplate)
        {
            Debug.Assert(!methodDef.IsConstructor);
            Debug.Assert(!methodDef.IsStatic);
            Debug.Assert(methodDef.IsAbstract);

            var baseMethodDefs = typeDef.Interfaces.SelectMany(id => id.Resolve().Methods)
                                        .Concat((typeDef.BaseType ?? TypeRefs.Object).Resolve().Methods.Where(md => md.IsVirtual));


            var baseMethod = GetPossibleBaseMethod(baseMethodDefs, methodDef);

            methodDef.IsVirtual = baseMethod != null && baseMethod.Resolve().IsVirtual;
            methodDef.IsHideBySig = true;
            methodDef.IsAbstract = false;
            methodDef.IsNewSlot = false;
            
            var nativeSignature = new NativeSignature(methodDef);            
            cppHeaderTemplate.Add(nativeSignature);            

            var ilp = methodDef.Body.GetILProcessor();
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldfld, cppThisPtr);

            if (nativeSignature.HasLargeReturn)
                ilp.Emit(OpCodes.Ldloca, nativeSignature.LargeReturnLocal);

            for (int j = 0; j < methodDef.Parameters.Count; ++j)
                if (methodDef.Parameters[j].ParameterType.CanBePassedAroundInRegister())
                    ilp.Emit(OpCodes.Ldarg, j + 1);
                else
                    ilp.Emit(OpCodes.Ldarga, j + 1);

            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldfld, cppThisPtr);
            ilp.Emit(OpCodes.Ldind_I);
            ilp.Emit(OpCodes.Ldc_I4, vTableSlotIdx * TypeRefs.SizeOfVTableSlot);
            ilp.Emit(OpCodes.Conv_I);
            ilp.Emit(OpCodes.Add);
            ilp.Emit(OpCodes.Ldind_I);

            ilp.Emit(OpCodes.Calli, nativeSignature.CallSite);
            
            if (nativeSignature.HasLargeReturn)
                ilp.Emit(OpCodes.Ldloc, nativeSignature.LargeReturnLocal.Index);

            ilp.Emit(OpCodes.Ret);

            methodDef.Body.OptimizeMacros();
        }

        private static MethodReference GetPossibleBaseMethod(IEnumerable<MethodReference> candidateRefs, MethodReference overrideRef)
		{
			foreach (var candidateRef in candidateRefs)
            {
				if (candidateRef.Name != overrideRef.Name)
					continue;

				if (candidateRef.HasGenericParameters != overrideRef.HasGenericParameters)
					continue;

				if (candidateRef.HasGenericParameters && candidateRef.GenericParameters.Count != overrideRef.GenericParameters.Count)
					continue;

				if (!AreSame(candidateRef.ReturnType, overrideRef.ReturnType))
					continue;

				if (candidateRef.HasParameters != overrideRef.HasParameters)
					continue;

				if (!candidateRef.HasParameters && !overrideRef.HasParameters)
					return candidateRef;

				if (!AreSame (candidateRef.Parameters, overrideRef.Parameters))
					continue;

				return candidateRef;
			}

			return null;
		}

        private static bool AreSame(Collection<ParameterDefinition> a, Collection<ParameterDefinition> b)
        {
            var count = a.Count;

            if (count != b.Count)
                return false;

            if (count == 0)
                return true;

            for (int i = 0; i < count; i++)
                if (!AreSame(a[i].ParameterType, b[i].ParameterType))
                    return false;

            return true;
        }

        private static bool AreSame(TypeReference a, TypeReference b)
        {
            return a.FullName == b.FullName;
        }
}
}
