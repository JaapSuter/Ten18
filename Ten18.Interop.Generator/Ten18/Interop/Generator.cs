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

namespace Ten18.Interop
{
    class Generator
    {
        public Generator(Assembly assembly)
        {
            mAssembly = assembly;
            mAssemblyDir = Path.GetDirectoryName(assembly.Location);

            var publicKeyResourceNameEndsWith = "Ten18.snk";
            var publicKeyData = GetEmbeddedResourceData(publicKeyResourceNameEndsWith);

            mAssemblyName = new AssemblyName(assembly.GetName().Name + ".Interop");
            mAssemblyFileName = mAssemblyName.Name + ".dll";

            mAssemblyName.KeyPair = new StrongNameKeyPair(publicKeyData);
            mAssemblyName.ProcessorArchitecture = ProcessorArchitecture.X86;
            mAssemblyName.Version = mAssembly.GetName().Version;

            var tgs = from t in assembly.GetExportedTypes()
                      where !t.GetCustomAttributes(typeof(GenerateCppDefinitionAttribute), inherit: false).IsEmpty()
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
            var assemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly(mAssemblyName, AssemblyBuilderAccess.Save, mAssemblyDir);
            var moduleBuilder = assemblyBuilder.DefineDynamicModule(mAssemblyFileName, emitSymbolInfo: true);

            mTypeGenerators.Run(tg => tg.Generate(mAssemblyDir, moduleBuilder));

            assemblyBuilder.Save(mAssemblyFileName, PortableExecutableKinds.Required32Bit | PortableExecutableKinds.ILOnly, ImageFileMachine.I386);

            var assemblyFullPath = Path.Combine(mAssemblyDir, mAssemblyFileName);

            AddDebugSymbols(assemblyFullPath);

            Console.WriteLine("Updated: {0}", assemblyFullPath);
        }

        private static void AddDebugSymbols(string assemblyFullPath)
        {
            // Todo, Jaap Suter, May 2011, hardcoded paths, dude...
            var key = @"D:\Projects\Code\Ten18\Code\Ten18.snk";
            var ildasm = @"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\NETFX 4.0 Tools\ildasm.exe";
            var ilasm = Path.Combine(RuntimeEnvironment.GetRuntimeDirectory(), "ilasm.exe");
            
            var psi0 = new ProcessStartInfo(ildasm, String.Format("{0} /OUT={0}.il /SOURCE", assemblyFullPath)) { UseShellExecute = false, CreateNoWindow = true, };
            Process.Start(psi0).WaitForExit();

            var psi1 = new ProcessStartInfo(ilasm, String.Format("{0}.il /OUTPUT={0} /DLL /DEBUG /KEY={1}", assemblyFullPath, key)) { UseShellExecute = false, CreateNoWindow = true, };
            Process.Start(psi1).WaitForExit();
        }

        private static byte[] GetEmbeddedResourceData(string nameEndsWith)
        {
            using (var ms = new MemoryStream())
            {
                var names = (from name in Assembly.GetExecutingAssembly().GetManifestResourceNames()
                             where name.EndsWith(nameEndsWith)
                             select name).ToArray();

                Debug.Assert(names.Length == 1);

                using (var rs = Assembly.GetExecutingAssembly().GetManifestResourceStream(names.First()))
                    rs.CopyTo(ms);

                return ms.GetBuffer();
            }
        }

        private readonly AssemblyName mAssemblyName;
        private readonly string mAssemblyFileName;
        private readonly string mAssemblyDir;

        private readonly Assembly mAssembly;
        
        private readonly TypeGenerator[] mTypeGenerators;
    }
}
