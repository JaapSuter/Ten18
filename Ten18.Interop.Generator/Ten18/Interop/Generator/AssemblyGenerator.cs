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
    class AssemblyGenerator
    {
        public AssemblyGenerator(Assembly assembly)
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

            mAssemblyBuilder = AppDomain.CurrentDomain.DefineDynamicAssembly(mAssemblyName, AssemblyBuilderAccess.Save, mAssemblyDir);
            mModuleBuilder = mAssemblyBuilder.DefineDynamicModule(mAssemblyFileName, emitSymbolInfo: true);
        }

        public void Generate()
        {
            NativeToManaged.Emit(mAssembly, mModuleBuilder, mAssemblyDir);
            ManagedToNative.Emit(mAssembly, mModuleBuilder, mAssemblyDir);

            mAssemblyBuilder.Save(mAssemblyFileName, PortableExecutableKinds.Required32Bit | PortableExecutableKinds.ILOnly, ImageFileMachine.I386);

            Console.WriteLine("Updated: {0}", Path.Combine(mAssemblyDir, mAssemblyFileName));
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
        private readonly AssemblyBuilder mAssemblyBuilder;
        private readonly ModuleBuilder mModuleBuilder;
    }
}
