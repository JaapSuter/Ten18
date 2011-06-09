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
using System.Runtime.CompilerServices;
using System.Security;

namespace Ten18.Interop
{
    class MethodGenerator
    {
        public string ExportName { get; private set; }
        public string NativeSignature { get; private set; }
        public int Ordinal { get; private set; }

        public string FullNameAsCpp()
        {
            return mPInvokeDef.FullNameAsCpp();
        }

        public string FullNameAsUndecorated()
        {
            return mPInvokeDef.FullNameAsUndecorated();
        }

        public MethodGenerator(MethodDefinition methodDef)
        {
            Debug.Assert(!methodDef.IsConstructor);
            Debug.Assert(methodDef.IsAbstract);

            mMethodDef = methodDef;

            var callConvAttribute = methodDef.IsStatic ? PInvokeAttributes.CallConvStdCall : PInvokeAttributes.CallConvThiscall;

            mPInvokeDef = new MethodDefinition(methodDef.Name, MethodAttributes.Private | MethodAttributes.PInvokeImpl | MethodAttributes.Static, methodDef.ReturnType);
            mPInvokeDef.DeclaringType = mMethodDef.DeclaringType;
            mPInvokeDef.IsPInvokeImpl = true;
            mPInvokeDef.IsPreserveSig = true;
            mPInvokeDef.IsHideBySig = true;
            mPInvokeDef.HasSecurity = true;
            mPInvokeDef.CustomAttributes.Add(new CustomAttribute(Globals.SupressUnmanagedCodeSecurityCtor));            
            mPInvokeDef.PInvokeInfo = new PInvokeInfo(PInvokeAttributes.BestFitDisabled
                                                    | PInvokeAttributes.CharSetUnicode
                                                    | PInvokeAttributes.NoMangle
                                                    | PInvokeAttributes.ThrowOnUnmappableCharDisabled
                                                    | callConvAttribute, String.Empty, Globals.PInvokeModuleRef);

            if (methodDef.IsStatic)
            {
                // There's no native this pointer to pass along...
            }
            else mPInvokeDef.Parameters.Add(new ParameterDefinition(Globals.NameOfCppThisPtrParameter, ParameterAttributes.In, Globals.VoidStar));

            foreach (var parameterDef in methodDef.Parameters)
            {
                var pInvokeParameterDef = parameterDef.ParameterType.CanBePassedAroundInRegister()
                                        ? new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType)
                                        : new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType.MakeByReferenceType());

                if (pInvokeParameterDef.ParameterType.IsSame(Globals.String))
                    pInvokeParameterDef.SetMarshalInfo(NativeType.LPWStr);

                if (pInvokeParameterDef.ParameterType.IsSame(Globals.Boolean))
                    pInvokeParameterDef.SetMarshalInfo(NativeType.U1);

                mPInvokeDef.Parameters.Add(pInvokeParameterDef);
            }

            if (mPInvokeDef.ReturnType.IsSame(Globals.Boolean))
            {
                mPInvokeDef.MethodReturnType.MarshalInfo = new MarshalInfo(NativeType.U1);
                mPInvokeDef.MethodReturnType.HasFieldMarshal = true;
            }
        }

        public void Generate(FieldDefinition cppThisPtr, CppHeaderTemplate cppHeaderTemplate)
        {
            mMethodDef.DeclaringType.Methods.Add(mPInvokeDef);
            mMethodDef.IsVirtual = ImplementsBaseOrInterface(mMethodDef);
            mMethodDef.IsFinal = mMethodDef.IsVirtual;
            mMethodDef.IsHideBySig = true;
            mMethodDef.IsAbstract = false;
            mMethodDef.IsNewSlot = false;

            VariableDefinition largeReturnLocalDef = null;
            if (!mPInvokeDef.ReturnType.CanBePassedAroundInRegister())
            {
                largeReturnLocalDef = new VariableDefinition("ret", mPInvokeDef.ReturnType);
                mPInvokeDef.ReturnType = Globals.Void;

                mMethodDef.Body.Variables.Add(largeReturnLocalDef);
                mMethodDef.Body.InitLocals = true;

                mPInvokeDef.Parameters.Add(new ParameterDefinition("ret", ParameterAttributes.Out | ParameterAttributes.Retval, largeReturnLocalDef.VariableType.MakeByReferenceType()));
            }

            var ilp = mMethodDef.Body.GetILProcessor();

            var paramOffsetForThisPtr = 1;
            if (mMethodDef.IsStatic)
            {
                paramOffsetForThisPtr = 0;
            }
            else
            {
                ilp.Emit(OpCodes.Ldarg_0);
                ilp.Emit(OpCodes.Ldfld, cppThisPtr);
            }

            for (int j = 0; j < mMethodDef.Parameters.Count; ++j)
                if (mMethodDef.Parameters[j].ParameterType.CanBePassedAroundInRegister())
                    ilp.Emit(OpCodes.Ldarg, j + paramOffsetForThisPtr);
                else
                    ilp.Emit(OpCodes.Ldarga, j + paramOffsetForThisPtr);

            if (largeReturnLocalDef != null)
                ilp.Emit(OpCodes.Ldloca, largeReturnLocalDef);

            ilp.Emit(OpCodes.Call, mMethodDef.Module.Import(mPInvokeDef));

            if (largeReturnLocalDef != null)
                ilp.Emit(OpCodes.Ldloc, largeReturnLocalDef);

            ilp.Emit(OpCodes.Ret);

            ilp.Body.OptimizeMacros();

            cppHeaderTemplate.Add(this);

            Ordinal = ExportTable.GetKnownOrRegisterForNewOrdinal(this);
            mPInvokeDef.PInvokeInfo.EntryPoint = String.Format("#{0}", Ordinal); ;
        }

        public static void PatchConstructor(TypeDefinition typeDef, MethodDefinition ctorDef, FieldDefinition cppThisPtr)
        {
            Debug.Assert(ctorDef.IsConstructor);

            // We're specifying this factory method as both abstract and static. That doesn't make sense in the CLR, but the method won't actually
            // be abstract then anymore, just static. Abstract is merely a marker that tells this program to provide an interop export
            // based implementation. Perhaps it'd be better to introduce a "CustomDllImportAttribute" type instead, but alas... no time.
            var methodDef = new MethodDefinition("New", MethodAttributes.Static | MethodAttributes.Abstract | MethodAttributes.Assembly, Globals.Void.MakePointerType());
            ctorDef.Parameters.Run(p => methodDef.Parameters.Add(new ParameterDefinition(p.Name, p.Attributes, p.ParameterType)));

            typeDef.Methods.Add(methodDef);

            var ilp = ctorDef.Body.GetILProcessor();
            var offset = ctorDef.Body.Instructions.First(i => i.OpCode == OpCodes.Call).Next;

            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarg_0));
            
            foreach (var parameterDef in methodDef.Parameters)
                ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarg, parameterDef));

            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Call, methodDef));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Stfld, cppThisPtr));

            ctorDef.Body.OptimizeMacros();
        }

        private static bool ImplementsBaseOrInterface(MethodDefinition methodDef)
        {
            var typeDef = methodDef.DeclaringType;

            var baseMethodDefs = typeDef.Interfaces.SelectMany(id => id.Resolve().Methods.Where(md => md.IsVirtual))
                                        .Concat((typeDef.BaseType ?? Globals.Object).Resolve().Methods.Where(md => md.IsVirtual));

            return null != GetPossibleBaseMethod(baseMethodDefs, methodDef);
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

                if (!candidateRef.ReturnType.IsSame(overrideRef.ReturnType))
                    continue;

                if (candidateRef.HasParameters != overrideRef.HasParameters)
                    continue;

                if (!candidateRef.HasParameters && !overrideRef.HasParameters)
                    return candidateRef;

                if (!candidateRef.Parameters.AreSameByType(overrideRef.Parameters))
                    continue;

                return candidateRef;
            }

            return null;
        }

        private MethodDefinition mMethodDef;
        private MethodDefinition mPInvokeDef;
    }
}
