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

namespace Ten18.Build
{
    static class Paths
    {
        public static string SolutionDir { get; private set; }
        public static string WorkingDir { get; private set; }
        public static string KeyFile { get; private set; }
        
        public static string WindowsSdkDir { get; private set; }
        public static string NetFxToolsDir { get; private set; }
        public static string DxSdkDir { get; private set; }

        public static string ILAsmExe { get; private set; }
        public static string ILDasmExe { get; private set; }
        public static string PEVerifyExe { get; private set; }
        public static string FxcExe { get; private set; }

        static Paths()
        {
            var exeDir = Path.GetDirectoryName(Assembly.GetEntryAssembly().Location);
            var slnDir = Path.GetDirectoryName(Path.GetDirectoryName(Path.GetDirectoryName(exeDir)));

            Debug.Assert(File.Exists(Path.Combine(slnDir, "Ten18.sln")));
            SolutionDir = slnDir;
            Console.WriteLine(SolutionDir);

            KeyFile = Path.Combine(slnDir, "Ten18.snk");
            Debug.Assert(File.Exists(KeyFile));

            WindowsSdkDir = Args.Get("WindowsSdkDir", Directory.Exists, () =>
                            new []
                            {
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), @"Microsoft SDKs\Windows\v7.0A\bin\"),
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), @"Microsoft SDKs\Windows\v7.0\bin\"),
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), @"Microsoft SDKs\Windows\v6.1\bin\"),
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),    @"Microsoft SDKs\Windows\v7.0A\bin\"),
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),    @"Microsoft SDKs\Windows\v7.0\bin\"),
                                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),    @"Microsoft SDKs\Windows\v6.1\bin\"),
                            }.FirstOrDefault(Directory.Exists));

            Debug.Assert(WindowsSdkDir != null);

            NetFxToolsDir = Path.Combine(WindowsSdkDir, @"NETFX 4.0 Tools");
            if (!Directory.Exists(NetFxToolsDir))
                NetFxToolsDir = WindowsSdkDir;

            ILAsmExe = Path.Combine(RuntimeEnvironment.GetRuntimeDirectory(), "ilasm.exe");
            ILDasmExe = Path.Combine(NetFxToolsDir, "ildasm.exe");
            PEVerifyExe = Path.Combine(NetFxToolsDir, "PEVerify.exe");

            WorkingDir = Args.Get<string>("WorkingDir", Directory.Exists, Environment.CurrentDirectory);

            DxSdkDir = Environment.GetEnvironmentVariable("DXSDK_DIR") ?? Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86), @"Microsoft DirectX SDK (June 2010)");
            FxcExe = Path.Combine(DxSdkDir + @"Utilities\bin\x86\fxc.exe");        
        }
    }
}
