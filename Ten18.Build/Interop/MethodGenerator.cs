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

namespace Ten18.Interop
{
    class MethodGenerator
    {
        public string ExportName { get; private set; }
        public string NativeSignature { get; private set; }
        public int Ordinal { get; private set; }

        public MethodGenerator(MethodDefinition methodDef, PatchTableTemplate patchTableTemplate)
        {
            Debug.Assert(!methodDef.IsConstructor);
            Debug.Assert(!methodDef.IsStatic);
            Debug.Assert(methodDef.IsAbstract);

            mMethodDef = methodDef;

            // Todo, Jaap Suter, May 2011, look into SuppressUnmanagedCodeSecurityAttribute

            Ordinal = patchTableTemplate.RegisterForOrdinal(this);

            mPInvokeDef = new MethodDefinition(methodDef.Name, MethodAttributes.Private | MethodAttributes.PInvokeImpl | MethodAttributes.Static, methodDef.ReturnType);
            mPInvokeDef.DeclaringType = mMethodDef.DeclaringType;
            mPInvokeDef.IsPInvokeImpl = true;
            mPInvokeDef.IsPreserveSig = true;
            mPInvokeDef.IsHideBySig = true;
            mPInvokeDef.PInvokeInfo = new PInvokeInfo(PInvokeAttributes.BestFitDisabled
                                                   | PInvokeAttributes.CallConvThiscall
                                                   | PInvokeAttributes.CharSetUnicode
                                                   | PInvokeAttributes.NoMangle
                                                   | PInvokeAttributes.ThrowOnUnmappableCharDisabled, String.Format("#{0}", Ordinal), TypeRefs.PInvokeModuleRef);

            mPInvokeDef.Parameters.Add(new ParameterDefinition("cppThisPtr", ParameterAttributes.In, TypeRefs.VoidStar));

            foreach (var parameterDef in methodDef.Parameters)
            {
                var pInvokeParameterDef = parameterDef.ParameterType.CanBePassedAroundInRegister()
                                        ? new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType)
                                        : new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType.MakeByReferenceType());

                if (pInvokeParameterDef.ParameterType.Resolve() == TypeRefs.String)
                    pInvokeParameterDef.MarshalInfo = new MarshalInfo(NativeType.LPWStr);

                if (pInvokeParameterDef.ParameterType.Resolve() == TypeRefs.Boolean)
                    pInvokeParameterDef.MarshalInfo = new MarshalInfo(NativeType.U1);

                mPInvokeDef.Parameters.Add(pInvokeParameterDef);
            }

            if (mPInvokeDef.ReturnType.Resolve() == TypeRefs.Boolean)
                mPInvokeDef.MethodReturnType.MarshalInfo = new MarshalInfo(NativeType.U1);
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
                mPInvokeDef.ReturnType = TypeRefs.Void;

                mMethodDef.Body.Variables.Add(largeReturnLocalDef);
                mMethodDef.Body.InitLocals = true;

                mPInvokeDef.Parameters.Add(new ParameterDefinition("ret", ParameterAttributes.Out | ParameterAttributes.Retval, largeReturnLocalDef.VariableType.MakeByReferenceType()));
            }

            var ilp = mMethodDef.Body.GetILProcessor();
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldfld, cppThisPtr);

            for (int j = 0; j < mMethodDef.Parameters.Count; ++j)
                if (mMethodDef.Parameters[j].ParameterType.CanBePassedAroundInRegister())
                    ilp.Emit(OpCodes.Ldarg, j + 1);
                else
                    ilp.Emit(OpCodes.Ldarga, j + 1);

            if (largeReturnLocalDef != null)
                ilp.Emit(OpCodes.Ldloca, largeReturnLocalDef);

            ilp.Emit(OpCodes.Call, mMethodDef.Module.Import(mPInvokeDef));

            if (largeReturnLocalDef != null)
                ilp.Emit(OpCodes.Ldloc, largeReturnLocalDef);

            ilp.Emit(OpCodes.Ret);

            ilp.Body.OptimizeMacros();

            InitializeExportNameAndNativeSignature();

            cppHeaderTemplate.Add(this);
        }

        private static Dictionary<int, string> sHack = new Dictionary<int, string>()
        { 
            {  9, "?Dispose@Window@Ten18@@QAEXXZ" },
            {  8, "?MakeFullScreen@Window@Ten18@@QAEXXZ" },
            { 10, "?NewInput@NativeFactory@Interop@Ten18@@QAEPAXXZ" },
            { 11, "?NewWindow@NativeFactory@Interop@Ten18@@QAEPAXPB_W@Z" },
            {  2, "?get_HasClosed@Window@Ten18@@QAE_NXZ" },
            {  7, "?get_IsFullScreen@Window@Ten18@@QAE_NXZ" },
            {  1, "?get_MousePosition@Input@1Ten18@@QAEXAAU_XMFLOAT2@@@Z" },
            {  5, "?get_Position@Window@Ten18@@QAEXAAU_XMFLOAT2@@@Z" },
            {  3, "?get_Size@Window@Ten18@@QAEXAAU_XMFLOAT2@@@Z" },
            {  6, "?set_Position@Window@Ten18@@QAEXABU_XMFLOAT2@@@Z" },
            {  4, "?set_Size@Window@Ten18@@QAEXABU_XMFLOAT2@@@Z" },
        };

        private void InitializeExportNameAndNativeSignature()
        {
            ExportName = sHack[Ordinal];

            NativeSignature = String.Format("{0} __thiscall {1}({2});",
                mPInvokeDef.ReturnType.FullNameInCpp(),
                mPInvokeDef.Name,
                String.Join(", ", mPInvokeDef.Parameters.Skip(1).Select(pd =>
                {
                    if (pd.ParameterType.Resolve() == TypeRefs.String)
                        return pd.ParameterType.FullNameInCpp() + " " + pd.Name;
                    else if (pd.ParameterType.IsByReference)
                        if (pd.IsOut)
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "& " + pd.Name;
                        else
                            return "const " + pd.ParameterType.GetElementType().FullNameInCpp() + "& " + pd.Name;
                    else if (pd.ParameterType.IsPointer)
                        if (pd.Attributes.HasFlag(ParameterAttributes.Out))
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                        else
                            return "const " + pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                    else
                        if (pd.ParameterType.IsPrimitive)
                            return pd.ParameterType.FullNameInCpp() + " " + pd.Name;
                        else
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                })));
        }

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
                                        .Concat((typeDef.BaseType ?? TypeRefs.Object).Resolve().Methods.Where(md => md.IsVirtual));

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

                if (!AreSame(candidateRef.ReturnType, overrideRef.ReturnType))
                    continue;

                if (candidateRef.HasParameters != overrideRef.HasParameters)
                    continue;

                if (!candidateRef.HasParameters && !overrideRef.HasParameters)
                    return candidateRef;

                if (!AreSame(candidateRef.Parameters, overrideRef.Parameters))
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

        private MethodDefinition mMethodDef;
        private MethodDefinition mPInvokeDef;
    }
}
