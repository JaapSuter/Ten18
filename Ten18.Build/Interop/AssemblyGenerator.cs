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
    class AssemblyGenerator
    {
        public AssemblyGenerator(string assemblyPath)
        {
            mAssemblyPath = assemblyPath;
            mAssemblyDef = AssemblyDefinition.ReadAssembly(mAssemblyPath, new ReaderParameters(ReadingMode.Immediate));
            mAssemblyDir = Path.GetDirectoryName(assemblyPath);

            InteropType.Initialize(mAssemblyDef.MainModule);

            foreach (var md in mAssemblyDef.Modules)
            {
                var requiredModuleAttributes = ModuleAttributes.ILOnly | ModuleAttributes.StrongNameSigned | ModuleAttributes.Required32Bit;
                Debug.Assert(md.Attributes == requiredModuleAttributes);
                mTypeGenerators.AddRange(from td in md.Types
                                         let tg = TypeGenerator.Create(td)
                                         where tg != null
                                         select tg);
            }
        }

        public void Generate()
        {
            mTypeGenerators.Run(tg => tg.Generate());

            InteropType.CompleteNativeTypeFactory(mAssemblyDef.MainModule);
            
            mAssemblyDef.MainModule.Architecture = TargetArchitecture.I386;

            var backupAssemblyPath = Path.ChangeExtension(mAssemblyPath, ".Backup.dll");
            if (File.Exists(backupAssemblyPath))
                File.Delete(backupAssemblyPath);
            File.Move(mAssemblyPath, backupAssemblyPath);

            mAssemblyDef.Write(mAssemblyPath, new WriterParameters { WriteSymbols = true, StrongNameKeyPair = new StrongNameKeyPair(File.ReadAllBytes(Paths.KeyFile)) });

            PostProcess(mAssemblyPath);

            Console.WriteLine("Updated: {0}", mAssemblyPath);
        }

        private static void PostProcess(string assemblyFullPath)
        {
            Paths.RunExe(Paths.ILDasmExe, "/SOURCE /OUT={0}.il {0}", assemblyFullPath);
            Paths.RunExe(Paths.ILAsmExe, "{0}.il /OUTPUT={0} /DLL /DEBUG /KEY={1}", assemblyFullPath, Paths.KeyFile);
            
            // Verify the MSIL, but ignore...
            //      * [found Int32] Expected ByRef on the stack.(Error: 0x80131860)
            Paths.RunExe(Paths.PEVerifyExe, "{0} /VERBOSE /NOLOGO /HRESULT /IGNORE=0x80131860", assemblyFullPath);
        }

        private readonly string mAssemblyDir;
        private readonly string mAssemblyPath;
        private readonly AssemblyDefinition mAssemblyDef;
        private readonly List<TypeGenerator> mTypeGenerators = new List<TypeGenerator>();
    }
}
