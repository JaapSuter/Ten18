using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Reflection;
using System.Diagnostics;
using Microsoft.Win32;
using System.ComponentModel;
using System.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace DllExport
{
    // Export native 64bit method from .NET assembly
    // =============================================
    // It is well known fact that .NET assembly could be tweaked to export native method, 
    // similar way how normal DLLs do it. There is good description and tool from Selvin 
    // for 32bit native function.
    //
    // My problem was how to do it for 64bits. Here you go.
    //
    // 1) you IlDAsm your assembly into il code.
    // 2) Edit the IL and change header to look like this:
    //
    // For 32bit:
    //   .corflags 0x00000002
    //   .vtfixup [1] int32 fromunmanaged at VT_01
    //   .data VT_01 = int32[1]
    //
    // For 64bit 
    //   .corflags 0x00000008 
    //   .vtfixup [1] int64 fromunmanaged at VT_01
    //   .data VT_01 = int64[1]
    //
    // 3) Header of your exported method would look similar to this. This is same for 32bit version. 
    //   .vtentry 1 : 1
    //   .export [1] as Java_net_sf_jni4net_Bridge_initDotNet
    //
    // 4) You IlAsm the file back into DLL. For x64 you use /x64 flag.
    //
    // 5) Update: It looks like none of the .vtfixup, .data or .vtentry changes are required any more to make this work.
    //  This simplifies the parser quite a lot. We only need to change .corflags and modify the method signature 

    /// <summary>
    /// Exports the specified methods as C callable functions
    /// </summary>
    class Exporter
    {
        #region Enums

        private enum Platform
        {
            x86,
            x64,
        }

        #endregion Enums

        #region Fields

        private string m_filePathNameExt;
        private string m_outPathName;
        private bool m_debug;
        private bool m_verbose;
        private bool m_argsValid;
        private List<string> m_lines = new List<string>();
        private int m_exportIdx;
        private string m_x86NameSuffix;
        private string m_x64NameSuffix;
        private bool m_exportX86;
        private bool m_exportX64;

        #endregion Fields

        #region Initialization

        /// <summary>
        /// Constructor
        /// </summary>
        public Exporter()
        {
        }

        #endregion Initialization

        #region Properties

        /// <summary>
        /// Get just the file name without extension
        /// </summary>
        private string FileName
        {
            get { return Path.GetFileNameWithoutExtension(m_filePathNameExt); }
        }

        /// <summary>
        /// Get just the file extension
        /// </summary>
        private string FileExtention
        {
            get { return Path.GetExtension(m_filePathNameExt); }
        }

        /// <summary>
        /// Get the file name and extension
        /// </summary>
        private string FileNameAndExtention
        {
            get { return Path.GetFileName(m_filePathNameExt); }
        }

        /// <summary>
        /// Get the folder that contains the file
        /// </summary>
        private string FileFolder
        {
            get { return Path.GetDirectoryName(m_filePathNameExt); }
        }

        /// <summary>
        /// Get the path to the disassembler
        /// </summary>
        private string DisassemblerPath
        {
            get
            {
                var registryPath = @"SOFTWARE\Microsoft\Microsoft SDKs\Windows";
                var registryValue = "CurrentInstallFolder";
                var key = Registry.LocalMachine.OpenSubKey(registryPath) ?? Registry.CurrentUser.OpenSubKey(registryPath);

                if (key == null)
                    throw new Exception("Cannot locate ildasm.exe.");

                var path = key.GetValue(registryValue) as string;

                if (path == null)
                    throw new Exception("Cannot locate ildasm.exe.");

                path = Path.Combine(path, @"Bin\ildasm.exe");

                if (!File.Exists(path))
                    throw new Exception("Cannot locate ildasm.exe.");

                return path;
            }
        }

        /// <summary>
        /// Get the path to the assembler
        /// </summary>
        private string AssemblerPath
        {
            get
            {
                var version = Environment.Version.Major.ToString() + "." +
                       Environment.Version.Minor.ToString() + "." +
                       Environment.Version.Build.ToString();

                var path = Environment.ExpandEnvironmentVariables(@"%SystemRoot%\Microsoft.NET\Framework\v" + version + @"\ilasm.exe");

                if (!File.Exists(path))
                    throw new Exception("Cannot locate ilasm.exe.");

                return path;
            }
        }

        #endregion Properties

        #region Public Methods

        /// <summary>
        /// Run the conversion
        /// </summary>
        public int Execute()
        {
            // This will cause an Assert dialog to be shown, at which point the debugger
            // can be attached in order to debug this code
            //Debug.Assert(false, "Exporter", "Exporter");

            ProcessArguments();

            if (!m_argsValid)
            {
                // Show usage
                Console.WriteLine("usage: DllExport.exe assembly [/Release|/Debug] [/Verbose] [/Out:new_assembly]");
                return 1;
            }

            if (!File.Exists(m_filePathNameExt))
                throw new Exception("The input file does not exist: '" + m_filePathNameExt + "'");

            WriteInfo("DllExport Tool");
            WriteInfo("Debug: " + m_debug);
            WriteInfo("Input: '" + m_filePathNameExt + "'");
            WriteInfo("Output: '" + m_outPathName + "'");

            Console.WriteLine("");

            Disassemble();
            ReadLines();
            //SaveLines(@"C:\Temp\DllExport\Disassembled Original.il");
            ParseAllDllExport();

            // 32-bit
            if (m_exportX86)
            {
                FixCorFlags(Platform.x86);
                Assemble(Platform.x86);
                //SaveLines(@"C:\Temp\DllExport\Disassembled x86.il");
            }

            // 64-bit
            if (m_exportX64)
            {
                FixCorFlags(Platform.x64);
                Assemble(Platform.x64);
                //SaveLines(@"C:\Temp\DllExport\Disassembled x64.il");
            }

            int exportCount = m_exportIdx - 1;

            Console.WriteLine("DllExport: Exported " + exportCount +
              (exportCount == 1 ? " function" : " functions"));

            Console.WriteLine();

            return 0;
        }

        #endregion Public Methods

        #region Private, Protected Methods

        /// <summary>
        /// Parse the arguments
        /// </summary>
        private void ProcessArguments()
        {
            m_debug = false;
            m_verbose = false;
            m_filePathNameExt = null;
            m_outPathName = null;
            m_x86NameSuffix = null;
            m_x64NameSuffix = null;
            m_exportX86 = false;
            m_exportX64 = false;

            string[] args = Environment.GetCommandLineArgs();

            for (int idx = 1; idx < args.Length; idx++)
            {
                string argLower = args[idx].ToLower();

                if (argLower.StartsWith("/name32:"))
                {
                    m_exportX86 = true;
                    m_x86NameSuffix = args[idx].Substring(8).Trim("\"".ToCharArray());
                }
                else if (argLower.StartsWith("/name64:"))
                {
                    m_exportX64 = true;
                    m_x64NameSuffix = args[idx].Substring(8).Trim("\"".ToCharArray());
                }
                else if (argLower == "/debug")
                {
                    m_debug = true;
                }
                else if (argLower == "/verbose")
                {
                    m_verbose = true;
                }
                else if (argLower.StartsWith("/input:"))
                {
                    m_filePathNameExt = args[idx].Substring(7).Trim();
                }
                else if (argLower.StartsWith("/output:"))
                {
                    m_outPathName = args[idx].Substring(8).Trim();
                }
            }

            if (!m_exportX86 && !m_exportX64)
                m_exportX86 = true;

            string path = Path.GetDirectoryName(m_filePathNameExt);

            if (!File.Exists(m_filePathNameExt) || this.FileFolder == string.Empty)
            {
                m_filePathNameExt = Path.Combine(Directory.GetCurrentDirectory(), m_filePathNameExt);

                if (!File.Exists(m_filePathNameExt))
                    throw new Exception("The input file does not exist: '" + m_filePathNameExt + "'");
            }

            if (string.IsNullOrEmpty(m_outPathName))
                m_outPathName = m_filePathNameExt;

            m_argsValid = !string.IsNullOrEmpty(m_filePathNameExt);
        }

        /// <summary>
        /// Disassemble the input file
        /// </summary>
        private void Disassemble()
        {
            m_exportIdx = 1;
            System.IO.Directory.SetCurrentDirectory(this.FileFolder);
            Process proc = new Process();

            // Must specify the /caverbal switch in order to get the custom attribute 
            // values as text and not as binary blobs
            string arguments = string.Format("/nobar{1}/out:\"{0}.il\" \"{0}.dll\"", this.FileName, " /linenum /caverbal ");

            WriteInfo("Disassemble file with arguments '" + arguments + "'");

            ProcessStartInfo info = new ProcessStartInfo(this.DisassemblerPath, arguments);

            info.UseShellExecute = false;
            info.CreateNoWindow = false;
            info.RedirectStandardOutput = true;
            proc.StartInfo = info;

            try
            {
                proc.Start();
            }
            catch (Win32Exception e)
            {
                bool handled = false;

                if (e.NativeErrorCode == 3)
                {
                    // try to check wow64 program files
                    string fn = info.FileName;

                    if (fn.Substring(1, 16).ToLower() == @":\program files\")
                    {
                        info.FileName = fn.Insert(16, " (x86)");
                        handled = true;
                        proc.Start();
                    }
                }
                if (!handled)
                    throw (e);
            }

            proc.WaitForExit();

            if (proc.ExitCode != 0)
            {
                WriteError(proc.StandardOutput.ReadToEnd());
                throw new Exception("Could not Disassemble: Error code '" + proc.ExitCode + "'");
            }
        }

        /// <summary>
        /// Read all the lines from the disassembled IL file
        /// </summary>
        private void ReadLines()
        {
            m_lines.Clear();

            if (string.IsNullOrEmpty(m_filePathNameExt))
                throw new Exception("The input file could not be found");

            string ilFile = Path.Combine(this.FileFolder, this.FileName + ".il");

            if (!File.Exists(ilFile))
                throw new Exception("The disassembled IL file could not be found");

            StreamReader sr = File.OpenText(ilFile);

            while (!sr.EndOfStream)
            {
                string line = sr.ReadLine();
                m_lines.Add(line);
            }

            sr.Close();
            sr.Dispose();
        }

        /// <summary>
        /// Save the current lines to the specified file
        /// </summary>
        /// <param name="p"></param>
        private void SaveLines(string fileName)
        {
            try
            {
                var folder = Path.GetDirectoryName(fileName);

                if (!Directory.Exists(folder))
                    Directory.CreateDirectory(folder);

                var fileStream = File.CreateText(fileName);

                foreach (string line in m_lines)
                    fileStream.WriteLine(line);

                fileStream.Close();
            }
            catch (System.Exception)
            {
            }
        }

        /// <summary>
        /// Fix the Cor flags
        /// </summary>
        private void FixCorFlags(Platform platform)
        {
            for (int idx = 0; idx < m_lines.Count; idx++)
            {
                if (m_lines[idx].StartsWith(".corflags"))
                {
                    switch (platform)
                    {
                        case Platform.x86:
                            m_lines[idx] = ".corflags 0x00000002  // 32BITREQUIRED";
                            break;

                        case Platform.x64:
                            m_lines[idx] = ".corflags 0x00000008  // 64BITREQUIRED";
                            break;
                    }
                    break;
                }
            }
        }

        /// <summary>
        /// Parse all DllExport entries
        /// </summary>
        private void ParseAllDllExport()
        {
            string findString = ".custom instance void DllExport";

            int dllExportIdx = FindLineStartsWith(findString, true, -1, -1);

            while (dllExportIdx >= 0)
            {
                ParseDllExport(dllExportIdx);
                dllExportIdx = FindLineStartsWith(findString, true, dllExportIdx + 1, -1);
            }
        }

        /// <summary>
        /// Parse the DllExport entry
        /// </summary>
        /// <param name="dllExportIdx"></param>
        private void ParseDllExport(int dllExportIdx)
        {
            int exportNameIdx = FindLineContains("string('", true, dllExportIdx, dllExportIdx + 5);
            int calConvIdx = FindLineContains("int32(", true, dllExportIdx, dllExportIdx + 5);
            string exportName = null;
            int startIdx = 0;
            int endIdx = 0;

            if (calConvIdx < 0)
                throw new Exception("Could not find Calling Convention for line " + dllExportIdx.ToString());

            if (exportNameIdx >= 0)
            {
                startIdx = m_lines[exportNameIdx].IndexOf("('");
                endIdx = m_lines[exportNameIdx].IndexOf("')");

                if (startIdx >= 0 && endIdx >= 0)
                    exportName = m_lines[exportNameIdx].Substring(startIdx + 2, endIdx - startIdx - 2);
            }

            startIdx = m_lines[calConvIdx].IndexOf("int32(");
            endIdx = m_lines[calConvIdx].IndexOf(")");

            if (startIdx < 0 || endIdx < 0)
                throw new Exception("Could not find Calling Convention for line " + dllExportIdx.ToString());

            string calConvText = m_lines[calConvIdx].Substring(startIdx + 6, endIdx - startIdx - 6);
            int calConvValue = 0;

            if (!int.TryParse(calConvText, out calConvValue))
                throw new Exception("Could not parse Calling Convention for line " + dllExportIdx.ToString());

            CallingConvention callConv = (CallingConvention)calConvValue;

            int endDllExport = FindLineContains("}", true, calConvIdx, calConvIdx + 10);

            if (endDllExport < 0)
                throw new Exception("Could not find end of Calling Convention for line " + dllExportIdx.ToString());

            // Remove the DllExport lines
            while (endDllExport >= dllExportIdx)
                m_lines.RemoveAt(endDllExport--);

            int insertIdx = FindLineStartsWith(".maxstack", true, dllExportIdx, dllExportIdx + 20);

            if (insertIdx < 0)
                throw new Exception("Could not find '.maxstack' insert location for line " + dllExportIdx.ToString());

            int tabs = m_lines[insertIdx].IndexOf(".");

            string exportText = TabString(tabs) + ".export [" + (m_exportIdx++).ToString() + "]";

            if (!string.IsNullOrEmpty(exportName))
                exportText += " as " + exportName;

            m_lines.Insert(insertIdx, exportText);

            string methodName = UpdateMethodCalConv(FindLineStartsWith(".method", false, insertIdx - 1, -1), callConv);

            if (!string.IsNullOrEmpty(methodName))
            {
                if (!string.IsNullOrEmpty(exportName))
                    Console.WriteLine("Exported '" + methodName + "' as '" + exportName + "'");
                else
                    Console.WriteLine("Exported '" + methodName + "'");
            }
        }

        /// <summary>
        /// Update the method's calling convention
        /// </summary>
        /// <param name="methodIdx"></param>
        /// <param name="callConv"></param>
        private string UpdateMethodCalConv(int methodIdx, CallingConvention callConv)
        {
            if (methodIdx < 0 || FindLineStartsWith(".method", true, methodIdx, methodIdx) != methodIdx)
                throw new Exception("Invalid method index: " + methodIdx.ToString());

            int endIdx = FindLineStartsWith("{", true, methodIdx, -1);

            if (endIdx < 0)
                throw new Exception("Could not find method open brace location for line " + methodIdx.ToString());

            endIdx--;
            int insertLine = -1;
            int insertCol = -1;
            string methodName = null;

            for (int idx = methodIdx; idx <= endIdx; idx++)
            {
                int marshalIdx = m_lines[idx].IndexOf("marshal(");

                if (marshalIdx >= 0)
                {
                    // Must be inserted before the "marshal(" entry
                    insertLine = idx;
                    insertCol = marshalIdx;
                    break;
                }
                else
                {
                    int openBraceIdx = m_lines[idx].IndexOf('(');

                    while (openBraceIdx >= 0 && insertLine < 0 && insertCol < 0)
                    {
                        int spaceIdx = m_lines[idx].LastIndexOf(' ', openBraceIdx);

                        if (spaceIdx >= 0)
                        {
                            string findMethodName = m_lines[idx].Substring(spaceIdx + 1, openBraceIdx - spaceIdx - 1);

                            // The method name is anything but "marshal"
                            if (findMethodName != "marshal")
                            {
                                insertLine = idx;
                                insertCol = spaceIdx + 1;
                                methodName = findMethodName;
                                break;
                            }

                            openBraceIdx = m_lines[idx].IndexOf('(', openBraceIdx + 1);
                        }
                    }
                }

                if (methodIdx >= 0 && insertCol >= 0)
                    break;
            }

            if (insertLine < 0 || insertCol < 0)
                throw new Exception("Could not find method name for line " + methodIdx.ToString());

            string leftText = m_lines[insertLine].Substring(0, insertCol);
            string rightText = m_lines[insertLine].Substring(insertCol);
            string callConvText = "modopt([mscorlib]";

            switch (callConv)
            {
                case System.Runtime.InteropServices.CallingConvention.Cdecl:
                    callConvText += typeof(CallConvCdecl).FullName + ") ";
                    break;

                case System.Runtime.InteropServices.CallingConvention.FastCall:
                    callConvText += typeof(CallConvFastcall).FullName + ") ";
                    break;

                case System.Runtime.InteropServices.CallingConvention.StdCall:
                    callConvText += typeof(CallConvStdcall).FullName + ") ";
                    break;

                case System.Runtime.InteropServices.CallingConvention.ThisCall:
                    callConvText += typeof(CallConvThiscall).FullName + ") ";
                    break;

                case System.Runtime.InteropServices.CallingConvention.Winapi:
                    callConvText += typeof(CallConvStdcall).FullName + ") ";
                    break;

                default:
                    throw new Exception("Invalid calling convention specified: '" + callConv.ToString() + "'");
            }

            m_lines[insertLine] = leftText + callConvText + rightText;
            return methodName;
        }

        /// <summary>
        /// Assemble the destination file
        /// </summary>
        private void Assemble(Platform platform)
        {
            StreamWriter sw = File.CreateText(Path.Combine(this.FileFolder, this.FileName + ".il"));

            foreach (string line in m_lines)
                sw.WriteLine(line);

            sw.Close();
            sw.Dispose();

            string resFile = Path.Combine(this.FileFolder, this.FileName + ".res");
            string res = "\"" + resFile + "\"";

            if (File.Exists(resFile))
                res = " /resource=" + res;
            else
                res = "";

            Process proc = new Process();
            string extension = Path.GetExtension(m_filePathNameExt);
            string outFile = Path.Combine(Path.GetDirectoryName(m_outPathName),
              Path.GetFileNameWithoutExtension(m_outPathName));

            switch (platform)
            {
                case Platform.x86:
                    if (!string.IsNullOrEmpty(m_x86NameSuffix))
                        outFile += m_x86NameSuffix;
                    break;

                case Platform.x64:
                    if (!string.IsNullOrEmpty(m_x64NameSuffix))
                        outFile += m_x64NameSuffix;

                    break;
            }

            if (extension == string.Empty)
                extension = ".dll";

            outFile += extension;

            string argOptions = "/nologo /quiet /DLL";
            string argIl = "\"" + Path.Combine(this.FileFolder, this.FileName) + ".il\"";
            string argOut = "/out:\"" + outFile + "\"";

            if (m_debug)
                argOptions += " /debug /pdb";
            else
                argOptions += " /optimize";

            if (platform == Platform.x64)
                argOptions += " /x64";

            string arguments = argOptions + " " + argIl + " " + res + " " + argOut;

            WriteInfo("Compiling file with arguments '" + arguments + "'");

            ProcessStartInfo info = new ProcessStartInfo(this.AssemblerPath, arguments);
            info.UseShellExecute = false;
            info.CreateNoWindow = false;
            info.RedirectStandardOutput = true;
            proc.StartInfo = info;
            proc.Start();
            proc.WaitForExit();

            WriteInfo(proc.StandardOutput.ReadToEnd());

            if (proc.ExitCode != 0)
                throw new Exception("Could not Assemble: Error code '" + proc.ExitCode + "'");
        }

        /// <summary>
        /// Find the next line that starts with the specified text, ignoring leading whitespace
        /// </summary>
        /// <param name="findText"></param>
        /// <param name="startIdx"></param>
        /// <param name="endIdx"></param>
        /// <returns></returns>
        private int FindLineStartsWith(string findText, bool forward, int startIdx, int endIdx)
        {
            if (forward)
            {
                if (startIdx < 0)
                    startIdx = 0;

                if (endIdx < 0)
                    endIdx = m_lines.Count - 1;
                else
                    endIdx = Math.Min(endIdx, m_lines.Count - 1);

                for (int idx = startIdx; idx <= endIdx; idx++)
                {
                    if (m_lines[idx].Contains(findText) && m_lines[idx].Trim().StartsWith(findText))
                        return idx;
                }
            }
            else
            {
                if (startIdx < 0)
                    startIdx = m_lines.Count - 1;

                if (endIdx < 0)
                    endIdx = 0;

                for (int idx = startIdx; idx >= endIdx; idx--)
                {
                    if (m_lines[idx].Contains(findText) && m_lines[idx].Trim().StartsWith(findText))
                        return idx;
                }
            }

            return -1;
        }

        /// <summary>
        /// Find the line that contains the specified text
        /// </summary>
        /// <param name="findText"></param>
        /// <param name="startIdx"></param>
        /// <param name="endIdx"></param>
        /// <returns></returns>
        private int FindLineContains(string findText, bool forward, int startIdx, int endIdx)
        {
            if (forward)
            {
                if (startIdx < 0)
                    startIdx = 0;

                if (endIdx < 0)
                    endIdx = m_lines.Count - 1;
                else
                    endIdx = Math.Min(endIdx, m_lines.Count - 1);

                for (int idx = startIdx; idx < endIdx; idx++)
                {
                    if (m_lines[idx].Contains(findText))
                        return idx;
                }
            }
            else
            {
                if (startIdx < 0)
                    startIdx = m_lines.Count - 1;

                if (endIdx < 0)
                    endIdx = 0;

                for (int idx = startIdx; idx >= endIdx; idx--)
                {
                    if (m_lines[idx].Contains(findText))
                        return idx;
                }
            }

            return -1;
        }

        /// <summary>
        /// Get a string padded with the number of spaces
        /// </summary>
        /// <param name="tabCount"></param>
        /// <returns></returns>
        private string TabString(int tabCount)
        {
            if (tabCount <= 0)
                return string.Empty;

            StringBuilder sb = new StringBuilder();

            sb.Append(' ', tabCount);
            return sb.ToString();
        }

        /// <summary>
        /// Write an informational message
        /// </summary>
        /// <param name="info"></param>
        private void WriteInfo(string info)
        {
            if (m_verbose)
                Console.WriteLine(info);
        }

        /// <summary>
        /// Write an informational message
        /// </summary>
        /// <param name="info"></param>
        private void WriteError(string error)
        {
            Console.WriteLine(error);
        }

        #endregion Private, Protected Methods
    }
}
