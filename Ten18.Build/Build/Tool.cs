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
    static class Tool
    {
        public static string Run(string exe, string workingDir, string argsFmt, params string[] fmtArgs)        
        {
            // You should ask Jaap Suter why he wrote this the way it is... (think buffer full deadlock and
            // don't care about stderr completion on error). Wasteful? Sue me, gotta pick
            // my battles... sigh. Stringbuilder...Schwingbuilder, at least it's mildly interesting and 2:07 AM.
            var stderr = "";
            var stdout = "";
            var maxTimeOutMs = 12000;
            var proc = new Process()
            {            
                StartInfo = new ProcessStartInfo(exe, String.Format(argsFmt, fmtArgs))
                {
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    WorkingDirectory = workingDir,
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

            return stdout + Environment.NewLine + stderr;
        }        
    }
}
