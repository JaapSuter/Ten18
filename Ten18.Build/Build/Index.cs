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
            sEntries.Add(Asciify(name), path);
        }

        public static void GenerateHeaders()
        {
            var dir = Path.Combine(Args.Get<string>("WorkingDir"), "Ten18/Content");
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            
            var header = Path.Combine(dir, "Index.Generated.h");            
            using (var tw = File.CreateText(header))
            {
                tw.WriteLine("#include \"Ten18/Content/Index.h\"");
                tw.WriteLine();
                tw.WriteLine("namespace Ten18 { namespace Content {");
                tw.WriteLine();

                sEntries.Values.Select((path, idx) =>
                {
                    var bytes = File.ReadAllBytes(path);

                    var isConst = NeedsInteropPatch(path) ? " " : "const ";
                    var code = new StringBuilder();
                    bytes.Run(b => code.Append((sbyte)b).Append(", "));
                    tw.WriteLine("static __declspec(align(16)) {0}char DataForEntry{1:000}[{2:000}] = {{ {3} }};", isConst, idx, bytes.Length, code);
                    return default(Unit);

                }).Run();
                
                tw.WriteLine("static const int NumEntries = {0};", sEntries.Count);
                tw.WriteLine("static const Index::Entry Table[NumEntries] = {");
                
                sEntries.Keys.Select((name, idx) =>
                {
                    tw.WriteLine("    {{ \"{0}\", sizeof(DataForEntry{1:000}), DataForEntry{1:000}, {2} }},", name, idx, NeedsInteropPatch(name) ? "true" : "false");
                    return default(Unit);
                }).Run();

                tw.WriteLine("};");
                tw.WriteLine("");
                tw.WriteLine("}}");
                tw.WriteLine("");
            }
        }

        private static string Asciify(string str)
        {
            byte[] ascii = Encoding.Convert(Encoding.Unicode, Encoding.ASCII, Encoding.Unicode.GetBytes(str));
            return new string(Encoding.ASCII.GetChars(ascii));
        }

        private static bool NeedsInteropPatch(string str)
        {
            return str.ToUpperInvariant().Contains("Ten18.Interop.Generated".ToUpperInvariant());
        }

        private static Dictionary<string, string> sEntries = new Dictionary<string, string>();
    }
}
