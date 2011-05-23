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
using Mono.Cecil;

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
                    Console.WriteLine(e);
                    return -1;
                }
    
            return 0;
        }

        private static void MaybeThrowingMain()
        {
            var interopAssemblyPath = Args.Get<string>("ImplementInteropAssembly");
            if (interopAssemblyPath != null)
                ImplementInteropAssembly(interopAssemblyPath);

            if (Args.Get<bool>("EmbedContent"))
            {
                CompileShaders();
                EmbedAssemblies();
                Build.Index.Add("Ten18.Content.Images.Panorama", Path.Combine(Paths.SolutionDir, @"Ten18.Content\Images\Panorama.jpg"));
                Build.Index.GenerateHeaders();
            }
        }

        private static void ImplementInteropAssembly(string assemblyPath)
        {
            Console.WriteLine("interopAssemblyPath: " + assemblyPath);
            
            var assemblyGenerator = new AssemblyGenerator(assemblyPath);            
            assemblyGenerator.Generate();
        }

        private static void CompileShaders()
        {
            string flags = "/nologo /WX /Ges /Od /Op /O0 /Zi /Gdp ";
            // Todo, Jaap Suter, May 2011, release flags:   /nologo /WX /Ges /O3 /Qstrip_reflect /Qstrip_debug

            var dir = Path.Combine(Args.Get<string>("WorkingDir"), "Ten18/Content/Shaders");
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            foreach (var shader in Directory.EnumerateFiles(Path.Combine(Paths.SolutionDir, "Ten18.Content/Shaders"), "*.fx"))
            {
                var name = Path.GetFileNameWithoutExtension(shader);

                var vso = Path.Combine(dir, name + ".vso");
                var pso = Path.Combine(dir, name + ".pso");

                Paths.RunExe(Paths.FxcExe, "{0} /Tvs_4_0 /EVS /Fo{1} {2}", flags, vso, shader);
                Paths.RunExe(Paths.FxcExe, "{0} /Tps_4_0 /EPS /Fo{1} {2}", flags, pso, shader);

                Build.Index.Add("Ten18.Content.Shaders." + name + ".VS", vso);
                Build.Index.Add("Ten18.Content.Shaders." + name + ".PS", pso);
            }
        }

        private static void EmbedAssemblies()
        {
            EmbedAssembly("Ten18.Interop");
            EmbedAssembly("Ten18.Net");
            EmbedAssembly("SlimMath");
            EmbedAssembly("AsyncCtpLibrary");
        }

        private static void EmbedAssembly(string name)
        {
            var path = Path.Combine(Paths.WorkingDir, name + ".dll");
            var assemblyDef = AssemblyDefinition.ReadAssembly(path);
            Debug.Assert(1 == assemblyDef.Modules.Count);
            var postPolicy = (assemblyDef.MainModule.Attributes.HasFlag(ModuleAttributes.Required32Bit) && assemblyDef.MainModule.Architecture == TargetArchitecture.I386)
                            ? ", processorarchitecture=x86"
                            : ", processorarchitecture=msil";
            var assemblyName = AssemblyName.GetAssemblyName(path);
            var contentName = assemblyName.FullName + postPolicy;

            Console.WriteLine("Content: {0} at {1}", contentName, path);
            Build.Index.Add(contentName, path);
        }
    }
}
