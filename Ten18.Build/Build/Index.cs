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
using System.Text;

namespace Ten18.Build
{
    static class Index
    {
        public static void Add(string name, string path)
        {
            sEntries.Add(Normalize(name), path);
        }

        public static void Generate(string generatedHeaderFilePath)
        {
            using (var tw = File.CreateText(generatedHeaderFilePath))
            {
                tw.WriteLine("#include \"Ten18/Content/Index.h\"");
                tw.WriteLine();
                tw.WriteLine("namespace Ten18 { namespace Content {");
                tw.WriteLine();

                sEntries.Values.Select((path, idx) =>
                {
                    var bytes = File.ReadAllBytes(path);
                    var code = new StringBuilder();
                    bytes.Run(b => code.Append((sbyte)b).Append(", "));
                    tw.WriteLine("static __declspec(align(16)) const char DataForEntry{0:000}[{1:000}] = {{ {2} }};", idx, bytes.Length, code);
                    return default(Unit);

                }).Run();
                
                tw.WriteLine("static const int NumEntries = {0};", sEntries.Count);
                tw.WriteLine("static const Index::Entry Table[NumEntries] = {");
                
                sEntries.Keys.Select((name, idx) =>
                {
                    tw.WriteLine("    {{ \"{0}\", sizeof(DataForEntry{1:000}), DataForEntry{1:000} }},", name, idx);
                    return default(Unit);
                }).Run();

                tw.WriteLine("};");
                tw.WriteLine("");
                tw.WriteLine("}}");
                tw.WriteLine("");
            }
        }

        private static string Normalize(string str)
        {
            byte[] ascii = Encoding.Convert(Encoding.Unicode, Encoding.ASCII, Encoding.Unicode.GetBytes(str));
            return new string(Encoding.ASCII.GetChars(ascii));
        }

        private static Dictionary<string, string> sEntries = new Dictionary<string, string>();
    }
}
