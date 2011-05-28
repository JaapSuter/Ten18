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
using Mono.Cecil;
using Mono.Cecil.Rocks;
using Mono.Cecil.Cil;

namespace Ten18.Interop
{
    class NativeSignature
    {
        public static readonly MethodCallingConvention NativeCallingConvention = MethodCallingConvention.ThisCall;
        public const string CppCallingConvention = "__thiscall";

        public bool HasLargeReturn { get { return LargeReturnLocal != null; } }
        public VariableDefinition LargeReturnLocal { get; private set; }
        public TypeReference ReturnType { get; private set; }
        public TypeReference DeclaringType { get; private set; }
        public MethodDefinition PInvokeDef { get; private set; }
        public CallSite CallSite { get; private set; }
        public string Name { get; private set; }

        private const ulong CookiePrefix = 0xBA5E101800000000;
        private static ulong sCookieCounter = CookiePrefix;
        public ulong Cookie { get; private set; }

        public NativeSignature(MethodDefinition methodDef)
        {
            Cookie = sCookieCounter++;
            Name = methodDef.Name;
            ReturnType = methodDef.ReturnType;
            DeclaringType = methodDef.DeclaringType;
            
            if (!ReturnType.CanBePassedAroundInRegister())
            {
                LargeReturnLocal = new VariableDefinition("ret", ReturnType);
                ReturnType = TypeRefs.Void;
                
                methodDef.Body.Variables.Add(LargeReturnLocal);
                methodDef.Body.InitLocals = true;
            }

            CallSite = new CallSite(ReturnType) { CallingConvention = NativeCallingConvention, };
            CallSite.Parameters.Add(new ParameterDefinition("cppThisPtr", ParameterAttributes.In, TypeRefs.VoidStar));
            
            if (HasLargeReturn)
                CallSite.Parameters.Add(new ParameterDefinition("ret", ParameterAttributes.Out, LargeReturnLocal.VariableType.MakeByReferenceType()));

            foreach (var parameterDef in methodDef.Parameters)
                if (parameterDef.ParameterType.Resolve() == TypeRefs.String)
                    CallSite.Parameters.Add(new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, TypeRefs.Char.MakePointerType()));
                else if (parameterDef.ParameterType.CanBePassedAroundInRegister())
                    CallSite.Parameters.Add(new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType));
                else
                    CallSite.Parameters.Add(new ParameterDefinition(parameterDef.Name, parameterDef.Attributes | ParameterAttributes.In, parameterDef.ParameterType.MakeByReferenceType()));

            TypeRefs.PatchTable.Add(this);
        }

        public string NativeParameterListOf()
        {
            return String.Join(", ",
                CallSite.Parameters.Skip(1).Select(pd =>
                {
                    if (pd.ParameterType.IsByReference)
                        if (pd.IsOut)
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "& " + pd.Name;
                        else
                            return "const " + pd.ParameterType.GetElementType().FullNameInCpp() + "& " + pd.Name;
                    else if (pd.ParameterType.IsPointer)
                    {
                        if (pd.Attributes.HasFlag(ParameterAttributes.Out))
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                        else
                            return "const " + pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                    }
                    else
                        if (pd.ParameterType.IsPrimitive)
                            return pd.ParameterType.FullNameInCpp() + " " + pd.Name;
                        else
                            return pd.ParameterType.GetElementType().FullNameInCpp() + "* " + pd.Name;
                }));
        }
    }
}
