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
using System.Linq.Expressions;
using System.IO;
using Ten18.Interop;
using Ten18.Build;

namespace Ten18
{
    class EntryPoint
    {
        public static int Main(string[] args)
        {
            Console.WriteLine("Ten18.Build");

            Args.Set(args);

            if (Debugger.IsAttached)
                MaybeThrowingMain();
            else
                try
                {
                    MaybeThrowingMain();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                    return -1;
                }
    
            return 0;
        }

        private static void MaybeThrowingMain()
        {
            if (Args.Get<bool>("ImplementInteropAssembly"))
                ImplementInteropAssembly();

            if (Args.Get<bool>("EmbedContent"))
                EmbedAssemblies();            
        }

        private static void ImplementInteropAssembly()
        {
            var interopAssembly = typeof(IAppDomainManagerEx).Assembly;
            var assemblyGenerator = new AssemblyGenerator(interopAssembly);
            
            assemblyGenerator.Generate();
        }

        private static void EmbedAssemblies()
        {
            foreach (var dll in Directory.EnumerateFiles(Paths.WorkingDir, "*.dll", SearchOption.TopDirectoryOnly))
            {                
                var postPolicy = dll.Contains("AsyncCtpLibrary") ? ", processorarchitecture=msil" : ", processorarchitecture=x86";
                var assemblyName = AssemblyName.GetAssemblyName(dll);
                var contentName = assemblyName.FullName + postPolicy;

                Build.Index.Add(contentName, dll);
            }

            Build.Index.Add(Path.Combine(Paths.SolutionDir, @"Ten18.Content\Images\Panorama.jpg"), "Ten18.Content.Images.Panorama");
            Build.Index.Add(Path.Combine(Paths.SolutionDir, @"Ten18.Content\Shaders\Capture.pso"), "Ten18.Content.Shaders.Capture.PS");
            Build.Index.Add(Path.Combine(Paths.SolutionDir, @"Ten18.Content\Shaders\Capture.vso"), "Ten18.Content.Shaders.Capture.VS");
            Build.Index.Add(Path.Combine(Paths.SolutionDir, @"Ten18.Content\Shaders\GrayCode.pso"), "Ten18.Content.Shaders.GrayCode.PS");
            Build.Index.Add(Path.Combine(Paths.SolutionDir, @"Ten18.Content\Shaders\GrayCode.vso"), "Ten18.Content.Shaders.GrayCode.VS");

            var dir = Path.Combine(Paths.SolutionDir, @"obj\x86\Debug\Ten18\Content");
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            Build.Index.Generate(Path.Combine(dir, "Index.Generated.h"));
        }
    }
}
