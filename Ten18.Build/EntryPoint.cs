using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using Mono.Cecil;
using Ten18.Build;

namespace Ten18
{
    class EntryPoint
    {
        public static int Main(string[] args)
        {
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
                    Console.WriteLine("Ten18.Build ran into a problem:");
                    Console.WriteLine();
                    Console.WriteLine("\t" + e.Message.Replace("\n", "\n\t"));
                    Console.WriteLine();
                    Console.WriteLine("\t" + e.StackTrace.ToString().Replace("\n", "\n\t"));
                    return -1;
                }
    
            return 0;
        }

        private static void MaybeThrowingMain()
        {
            var debug = Args.Get("Debug", false) || Paths.WorkingDir.Contains("Debug");
            
            var interopAssemblyPath = Args.Get<string>("ImplementInteropAssembly");
            if (interopAssemblyPath != null)
                Interop.AssemblyGenerator.Generate(interopAssemblyPath, debug);

            if (Args.Get<bool>("UpdateNativeExportTable"))
                Interop.ExportTable.Update();

            if (Args.Get<bool>("ProcessData"))
            {
                CompileShaders(debug);
                                
                EmbedAssembly("Ten18.Net.Generated");
                EmbedAssembly("SlimMath");
                EmbedAssembly("AsyncCtpLibrary");

                EmbedImage("Ten18/Content/Images/Panorama.jpg");

                Build.Index.GenerateHeaders();
            }
        }

        private static void EmbedImage(string name)
        {
            var srcFile = Path.Combine(Paths.SolutionDir, name.Replace("Ten18/Content", "Ten18.Content"));
            var dstFile = Path.Combine(Paths.WorkingDir, name);

            if (File.GetLastWriteTime(dstFile) <= File.GetLastAccessTime(srcFile))
            {
                var dstDir = Path.GetDirectoryName(dstFile);
                if (!Directory.Exists(dstDir))
                    Directory.CreateDirectory(dstDir);
                File.Copy(srcFile, dstFile, true);
            }

            Build.Index.Add(dstFile);
        }
        
        private static void CompileShaders(bool debug)
        {
            string flags = debug
                         ? "/nologo /WX /Ges /Od /Op /O0 /Zi /Gdp "
                         : "/nologo /WX /Ges /O3 /Qstrip_reflect /Qstrip_debug";

            var dir = Path.Combine(Args.Get<string>("WorkingDir"), "Ten18/Content/Shaders");
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            foreach (var shader in Directory.EnumerateFiles(Path.Combine(Paths.SolutionDir, "Ten18.Content/Shaders"), "*.fx"))
            {
                var name = Path.GetFileNameWithoutExtension(shader);

                var vso = Path.Combine(dir, name + ".vso");
                var pso = Path.Combine(dir, name + ".pso");

                var needsUpdate = File.GetLastWriteTime(vso) <= File.GetLastWriteTime(shader)
                               || File.GetLastWriteTime(pso) <= File.GetLastWriteTime(shader);

                if (needsUpdate)
                {
                    Console.WriteLine("Updating Vertex Shader: {0}", vso);
                    Console.WriteLine("Updating Pixel Shader: {0}", pso);

                    Tool.Run(Paths.FxcExe, Paths.WorkingDir, "{0} /Tvs_4_0 /EVS /Fo{1} {2}", flags, vso, shader);
                    Tool.Run(Paths.FxcExe, Paths.WorkingDir, "{0} /Tps_4_0 /EPS /Fo{1} {2}", flags, pso, shader);                    
                }
                
                Build.Index.Add(vso);
                Build.Index.Add(pso);
            }
        }

        private static void EmbedAssembly(string name)
        {
            var path = Path.Combine(Paths.WorkingDir, name + ".dll");
            if (!File.Exists(path))
                Console.WriteLine("Unable to embed assembly, missing file: " + path);
            else
            {
                var assemblyDef = AssemblyDefinition.ReadAssembly(path);
                Debug.Assert(1 == assemblyDef.Modules.Count);
                var postPolicy = (assemblyDef.MainModule.Attributes.HasFlag(ModuleAttributes.Required32Bit) && assemblyDef.MainModule.Architecture == TargetArchitecture.I386)
                                ? ", processorarchitecture=x86"
                                : ", processorarchitecture=msil";
                var assemblyName = AssemblyName.GetAssemblyName(path);
                var contentName = assemblyName.FullName + postPolicy;

                Build.Index.Add(contentName, path);
            }
        }
    }
}
