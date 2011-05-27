using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;
using Mono.Cecil.Rocks;
using System.Collections.Generic;

namespace Ten18.Interop
{
    static class ClassGenerator
    {
        public static void Generate(TypeDefinition typeDef)
        {
            Debug.Assert(typeDef.IsClass);
            Debug.Assert(typeDef.IsAbstract);
            
            var cppThisPtr = new FieldDefinition("mCppThisPtr", FieldAttributes.InitOnly | FieldAttributes.Private, TypeRefs.VoidStar);
            typeDef.Fields.Add(cppThisPtr);
            typeDef.IsAbstract = false;
            typeDef.IsSealed = true;

            CreateExplicitDefaultConstructorIfNoneDefined(typeDef);

            if (typeDef == TypeRefs.NativeFactory)
                PatchNativeFactory(typeDef, cppThisPtr);
            else
                PatchConstructors(typeDef, cppThisPtr);

            var cppHeaderTemplate = new CppHeaderTemplate(typeDef);
            GenerateMethods(typeDef, cppThisPtr, cppHeaderTemplate);

            cppHeaderTemplate.Generate();
        }

        private static void CreateExplicitDefaultConstructorIfNoneDefined(TypeDefinition typeDef)
        {
            if (typeDef.Methods.Any(m => m.IsConstructor))
                return;
            
            var ctorDef = new MethodDefinition(".ctor", MethodAttributes.Public
                                                      | MethodAttributes.SpecialName
                                                      | MethodAttributes.RTSpecialName
                                                      | MethodAttributes.HideBySig, TypeRefs.Void) { HasThis = true, };

            var baseCtorRef = new MethodReference(".ctor", TypeRefs.Void, typeDef.BaseType ?? TypeRefs.Object) { HasThis = true };
            var ilp = ctorDef.Body.GetILProcessor();
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Call, baseCtorRef);
            ilp.Emit(OpCodes.Ret);
        }

        private static void PatchConstructors(TypeDefinition typeDef, FieldDefinition cppThisPtr)
        {
            Debug.Assert(typeDef != TypeRefs.NativeFactory);
            
            var ctorDefs = from methodDef in typeDef.Methods
                           where methodDef.IsConstructor
                           select methodDef;

            foreach (var ctorDef in ctorDefs.ToArray())
                MethodGenerator.PatchConstructor(typeDef, ctorDef, cppThisPtr); 
        }

        private static void PatchNativeFactory(TypeDefinition typeDef, FieldDefinition cppThisPtr)
        {
            Debug.Assert(typeDef == TypeRefs.NativeFactory);
            var ctorDef = typeDef.Methods.Single(methodDef => methodDef.IsConstructor);

            var paramDef = new ParameterDefinition("cppThisPtr", ParameterAttributes.In, TypeRefs.IntPtr);
            ctorDef.Parameters.Add(paramDef);
            ctorDef.IsPublic = true;

            var ilp = ctorDef.Body.GetILProcessor();
            var offset = ctorDef.Body.Instructions.First(i => i.OpCode == OpCodes.Call).Next;

            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarg_0));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Ldarga, paramDef));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Call, typeDef.Module.Import(typeof(IntPtr).GetMethod("ToPointer"))));
            ilp.InsertBefore(offset, Instruction.Create(OpCodes.Stfld, cppThisPtr));
            
            ctorDef.Body.OptimizeMacros();
        }

        private static void GenerateMethods(TypeDefinition typeDef, FieldDefinition cppThisPtr, CppHeaderTemplate cppHeaderTemplate)
        {
            int vTableSlotIdx = 0;            
            foreach (var methodDef in typeDef.Methods)
                if (methodDef.IsAbstract)
                    MethodGenerator.Generate(typeDef, methodDef, cppThisPtr, vTableSlotIdx++, cppHeaderTemplate);
        }
    }
}
