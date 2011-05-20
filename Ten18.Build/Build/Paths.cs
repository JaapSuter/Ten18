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

        public static string ILAsmExe { get; private set; }
        public static string ILDasmExe { get; private set; }
        public static string PEVerifyExe { get; private set; }

        public static void RunExe(string exe, string argsFmt, params string[] fmtArgs)
        {
            // You should ask Jaap Suter why he wrote this the way it is... (think buffer full deadlock and
            // don't care about stderr completion on error). Wasteful? Sue me, gotta pick
            // my battles... sigh. Stringbuilder...Schwingbuilder, at least it's mildly interesting and 2:07 AM.
            var stderr = "";
            var stdout = "";
            var maxTimeOutMs = 3000;
            var proc = new Process()
            {            
                StartInfo = new ProcessStartInfo(exe, String.Format(argsFmt, fmtArgs))
                {
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                },
            };
            
            proc.ErrorDataReceived += (object s, DataReceivedEventArgs e) => { stderr += e.Data + "\n"; };
            proc.OutputDataReceived += (object s, DataReceivedEventArgs e) => { stdout += e.Data + "\n"; };            
            
            proc.Start();
            
            proc.BeginErrorReadLine();
            proc.BeginOutputReadLine();
            var exited = proc.WaitForExit(maxTimeOutMs);
            proc.CancelOutputRead();
            proc.CancelErrorRead();
            if (!exited || proc.ExitCode != 0)
            {   
                Console.WriteLine(stdout + Environment.NewLine + stderr);
                throw exited ? new TimeoutException(String.Format("Child process failed (exit code = {0}: {1} {2})", proc.ExitCode, proc.StartInfo.FileName, proc.StartInfo.Arguments))
                             : new TimeoutException(String.Format("Child process timed out: {0} {1}", proc.StartInfo.FileName, proc.StartInfo.Arguments));
            }
        }
        
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
        }
    }
}
