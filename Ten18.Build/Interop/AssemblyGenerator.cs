using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using Ten18.Build;
using Mono.Cecil.Rocks;
using Mono.Cecil;
using FieldAttributes = Mono.Cecil.FieldAttributes;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ten18.Interop
{
    static class AssemblyGenerator
    {
        public static void Generate(string assemblyPath)
        {
            var generatedPath = Path.ChangeExtension(assemblyPath, ".Generated.dll");
            var assemblyDef = AssemblyDefinition.ReadAssembly(assemblyPath, new ReaderParameters(ReadingMode.Immediate));
            var moduleDef = assemblyDef.MainModule;

            var requiredModuleAttributes = ModuleAttributes.ILOnly | ModuleAttributes.StrongNameSigned | ModuleAttributes.Required32Bit;
            var requiredModuleArchitecture = TargetArchitecture.I386;
            Debug.Assert(moduleDef.Attributes == requiredModuleAttributes);
            Debug.Assert(moduleDef.Architecture == requiredModuleArchitecture);
            Debug.Assert(assemblyDef.Modules.Count == 1);

            TypeRefs.Initialize(moduleDef);

            var patchTableTemplate = new PatchTableTemplate();

            foreach (var typeDef in moduleDef.Types)
                if (typeDef == TypeRefs.NativeFactory) {} else
                if (typeDef.IsEnum) {} else
                if (typeDef.IsValueType) {} else
                if (typeDef.IsClass && typeDef.IsAbstract) ClassGenerator.Generate(typeDef, patchTableTemplate);

            ClassGenerator.Generate(TypeRefs.NativeFactory, patchTableTemplate);

            patchTableTemplate.Generate();

            assemblyDef.Name.Name = assemblyDef.Name.Name + ".Generated";
            assemblyDef.Write(generatedPath, new WriterParameters { 
                WriteSymbols = false,
                StrongNameKeyPair = new StrongNameKeyPair(File.ReadAllBytes(Paths.KeyFile)),
            });

            PostProcess(generatedPath);

            Console.WriteLine("Updated: {0}", generatedPath);
        }

        private static void PostProcess(string assemblyFullPath)
        {
            if (assemblyFullPath.Contains("Debug"))
            {
                Tool.Run(Paths.ILDasmExe, "/SOURCE /OUT={0}.il {0}", assemblyFullPath);
                Tool.Run(Paths.ILAsmExe, "{0}.il /OUTPUT={0} /DLL /DEBUG /KEY={1}", assemblyFullPath, Paths.KeyFile);
            }
            
            // Verify the MSIL, but ignore...
            //      * [IL]: Error: [found unmanaged pointer][expected unmanaged pointer] Unexpected type on the stack.(Error: 0x80131854)
            //      * [IL]: Error: [found unmanaged pointer] Expected ByRef on the stack.(Error: 0x80131860)
            //      * [IL]: Error: [found ref 'System.String'] Expected numeric type on the stack.(Error: 0x8013185D)
            //      * [IL]: Error: Instruction cannot be verified.(Error: 0x8013186E)
            Tool.Run(Paths.PEVerifyExe, "{0} /VERBOSE /NOLOGO /HRESULT /IGNORE=0x80131854,0x80131860,0x8013185D,0x8013186E", assemblyFullPath);
        }
    }
}
