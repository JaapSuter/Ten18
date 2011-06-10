using System;
using System.Diagnostics;

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

            var output = stdout + Environment.NewLine + stderr;
            if (!exited || proc.ExitCode != 0)
            {
                throw new Exception(String.Format("Child process {0}, exit code = {1}:\n\t{2} {3}\n\t{4}",
                            exited ? "failed" : "timed out",
                            proc.ExitCode,
                            proc.StartInfo.FileName,
                            proc.StartInfo.Arguments,
                            output.Replace("\n", "\n\t")));
            }

            return output;
        }        
    }
}
