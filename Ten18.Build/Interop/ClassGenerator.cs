using System.Diagnostics;
using System.Linq;
using Mono.Cecil;
using Mono.Cecil.Cil;

namespace Ten18.Interop
{
    static class ClassGenerator
    {
        public static void Generate(TypeDefinition typeDef)
        {
            Debug.Assert(typeDef.IsClass);
            Debug.Assert(typeDef.IsAbstract);

            var cppThisPtr = new FieldDefinition("mCppThisPtr", FieldAttributes.InitOnly | FieldAttributes.Private, Globals.VoidStar);
            typeDef.Fields.Add(cppThisPtr);
            typeDef.IsAbstract = false;
            typeDef.IsSealed = true;

            CreateExplicitDefaultConstructorIfNoneDefined(typeDef);

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
                                                      | MethodAttributes.HideBySig, Globals.Void) { HasThis = true, };

            var baseCtorRef = new MethodReference(".ctor", Globals.Void, typeDef.BaseType ?? Globals.Object) { HasThis = true };
            var ilp = ctorDef.Body.GetILProcessor();
            ilp.Emit(OpCodes.Ldarg_0);
            ilp.Emit(OpCodes.Call, baseCtorRef);
            ilp.Emit(OpCodes.Ret);
        }

        private static void PatchConstructors(TypeDefinition typeDef, FieldDefinition cppThisPtr)
        {
            var ctorDefs = from methodDef in typeDef.Methods
                           where methodDef.IsConstructor
                           select methodDef;

            foreach (var ctorDef in ctorDefs.ToArray())
                MethodGenerator.PatchConstructor(typeDef, ctorDef, cppThisPtr); 
        }

        private static void GenerateMethods(TypeDefinition typeDef, FieldDefinition cppThisPtr, CppHeaderTemplate cppHeaderTemplate)
        {
            var methodDefs = from methodDef in typeDef.Methods
                             where methodDef.IsAbstract
                             select methodDef;

            foreach (var methodDef in methodDefs.ToArray())
                new MethodGenerator(methodDef).Generate(cppThisPtr, cppHeaderTemplate);
        }
    }
}
