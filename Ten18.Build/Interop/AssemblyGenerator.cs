using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.Reflection.Emit;
using System.Reflection;
using System.CodeDom;
using System.CodeDom.Compiler;
using System.Runtime.CompilerServices;
using Microsoft.CSharp;
using Ten18.Interop;
using System.IO;
using Ten18.Build;

namespace Ten18.Interop
{
    class AssemblyGenerator
    {
        public AssemblyGenerator(Assembly assembly)
        {
            mAssembly = assembly;
            mAssemblyDir = Path.GetDirectoryName(assembly.Location);

            mAssemblyName = new AssemblyName(assembly.GetName().Name + ".Impl");
            mAssemblyName.ProcessorArchitecture = ProcessorArchitecture.X86;
            mAssemblyName.Version = mAssembly.GetName().Version;

            using (var keyFileStream = File.OpenRead(Paths.KeyFile))
                mAssemblyName.KeyPair = new StrongNameKeyPair(keyFileStream);
            
            var tgs = from t in assembly.GetExportedTypes()
                      let tg = t.IsEnum ? new EnumGenerator(t)
                             : t.IsValueType ? new StructGenerator(t)
                             : t.IsClass ? new ClassGenerator(t)
                             : (TypeGenerator)null
                      where tg != null
                      select tg;

            mTypeGenerators = tgs.ToArray();
        }

        public void Generate()
        {
            var assemblyFileName = mAssemblyName.Name + ".dll";
            var assemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly(mAssemblyName, AssemblyBuilderAccess.Save, mAssemblyDir);
            var moduleBuilder = assemblyBuilder.DefineDynamicModule(mAssemblyName.Name, assemblyFileName, emitSymbolInfo: true);

            mTypeGenerators.Run(tg => tg.Generate(mAssemblyDir, moduleBuilder));

            assemblyBuilder.Save(assemblyFileName, PortableExecutableKinds.Required32Bit | PortableExecutableKinds.ILOnly, ImageFileMachine.I386);

            var assemblyFullPath = Path.Combine(mAssemblyDir, assemblyFileName);

            PostProcess(assemblyFullPath);

            Console.WriteLine("Updated: {0}", assemblyFullPath);
        }

        private static void PostProcess(string assemblyFullPath)
        {
            Paths.RunExe(Paths.ILDasmExe, "/SOURCE /OUT={0}.il {0}", assemblyFullPath);
            Paths.RunExe(Paths.ILAsmExe, "{0}.il /OUTPUT={0} /DLL /DEBUG /KEY={1}", assemblyFullPath, Paths.KeyFile);
            
            // Verify the MSIL, but ignore...
            //      * [found Int32] Expected ByRef on the stack.(Error: 0x80131860)
            Paths.RunExe(Paths.PEVerifyExe, "{0} /VERBOSE /NOLOGO /HRESULT /IGNORE=0x80131860", assemblyFullPath);
        }

        private readonly AssemblyName mAssemblyName;
        private readonly string mAssemblyDir;

        private readonly Assembly mAssembly;
        
        private readonly TypeGenerator[] mTypeGenerators;
    }
}
