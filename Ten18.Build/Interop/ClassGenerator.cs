using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;
using Mono.Cecil.Rocks;

namespace Ten18.Interop
{
    class ClassGenerator : TypeGenerator
    {
        public ClassGenerator(TypeDefinition typeDef)
            : base(typeDef)
        {
            Debug.Assert(typeDef.IsClass);
            Debug.Assert(typeDef.IsAbstract);
            
            mMethodGenerators = typeDef.Methods
                                       .Where(md => md.IsAbstract)
                                       .Select((md, idx) => new MethodGenerator(md, idx))
                                       .ToArray();
        }

        protected override void GenerateCpp()
        {
            var cppHeaderTemplate = new CppHeaderTemplate()
            {
                InteropType = InteropType,
                MethodGenerators = mMethodGenerators,
            };

            var code = cppHeaderTemplate.TransformText();
            File.WriteAllText(CppHeaderFile, code);

            Console.WriteLine("Updated: {0}", CppHeaderFile);
        }

        protected override void GenerateCli()
        {
            TypeDef.IsAbstract = false;
            TypeDef.IsSealed = true;
            
            var cppThisPtrDef = new FieldDefinition("mCppThisPtr", FieldAttributes.InitOnly | FieldAttributes.Private, InteropType.CppThisPtrTypeRef);
            TypeDef.Fields.Add(cppThisPtrDef);
            
            if (TypeDef == InteropType.NativeTypeFactoryDef)
                CreateNativeTypeFactoryCtor(cppThisPtrDef);
            else
                TypeDef.Methods.Where(md => md.IsConstructor).Run(md => PatchCtor(md, cppThisPtrDef));

            mMethodGenerators.Run(mg => mg.GenerateCli(cppThisPtrDef));
        }

        private void CreateNativeTypeFactoryCtor(FieldDefinition cppThisPtrDef)
        {
            Debug.Assert(TypeDef == InteropType.NativeTypeFactoryDef);

            var ctorDef = new MethodDefinition(".ctor",
                MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.RTSpecialName | MethodAttributes.HideBySig, InteropType.VoidTypeRef);
            var paramDef = new ParameterDefinition("cppThisPtr", ParameterAttributes.In, cppThisPtrDef.FieldType);
            ctorDef.Parameters.Add(paramDef);

            var baseCtor = new MethodReference(".ctor", InteropType.VoidTypeRef, TypeDef.BaseType ?? InteropType.ObjectTypeRef) { HasThis = true };
            var ilp = ctorDef.Body.GetILProcessor();            
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Call, baseCtor);
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Ldarg_1);
            ilp.Emit(OpCodes.Stfld, cppThisPtrDef);
            ilp.Emit(OpCodes.Ret);
            
            TypeDef.Methods.Add(ctorDef);
        }

        private static void PatchCtor(MethodDefinition ctorDef, FieldDefinition cppThisPtrDef)
        {
            Debug.Assert(ctorDef.IsConstructor);
            ctorDef.IsPublic = true;

            var ilp = ctorDef.Body.GetILProcessor();
            var afterPatchInst = ctorDef.Body.Instructions.First(i => i.OpCode == OpCodes.Call).Next;

            var factoryMethodDef = new MethodDefinition("CreateNative" + ctorDef.DeclaringType.Name,
                MethodAttributes.Abstract | MethodAttributes.Assembly, InteropType.CppThisPtrTypeRef);
                        
            InteropType.NativeTypeFactoryDef.Methods.Add(factoryMethodDef);

            ilp.InsertBefore(afterPatchInst, Instruction.Create(OpCodes.Ldarg_0));
            ilp.InsertBefore(afterPatchInst, Instruction.Create(OpCodes.Ldsfld, InteropType.NativeTypeFactoryInstanceDef));
            ilp.InsertBefore(afterPatchInst, Instruction.Create(OpCodes.Call, factoryMethodDef));
            ilp.InsertBefore(afterPatchInst, Instruction.Create(OpCodes.Stfld, cppThisPtrDef));
        }

        private readonly MethodGenerator[] mMethodGenerators;
    }
}
