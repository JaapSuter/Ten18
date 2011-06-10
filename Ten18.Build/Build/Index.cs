using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace Ten18.Build
{
    static class Index
    {
        public static void Add(string path)
        {
            var normalized = Path.GetFullPath(path);
            var prefix = Path.GetFullPath(Paths.WorkingDir);
            if (!normalized.StartsWith(prefix))
                throw new Exception(
                    String.Format("Content that derives its name from its path must live in the working directory...({0}). So {1} is a no go...",
                        prefix, path));

            var name = normalized.Substring(prefix.Length).Replace('\\', '/');

            Add(name, path);
        }

        public static void Add(string name, string path)
        {
            if (sEmbedContent)
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

                    var code = new StringBuilder();
                    bytes.Run(b => code.Append((sbyte)b).Append(", "));
                    tw.WriteLine("static __declspec(align(16)) char DataForEntry{0:000}[{1:000}] = {{ {2} }};", idx, bytes.Length, code);
                    return default(Unit);

                }).Run();
                
                tw.WriteLine("static const int NumEntries = {0};", sEntries.Count);
                tw.WriteLine("static const Index::Entry Table[NumEntries + 1] = {");
                
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

        private static string Asciify(string str)
        {
            byte[] ascii = Encoding.Convert(Encoding.Unicode, Encoding.ASCII, Encoding.Unicode.GetBytes(str));
            return new string(Encoding.ASCII.GetChars(ascii));
        }

        private static readonly bool sEmbedContent = Args.Get<bool>("EmbedContent");

        private static Dictionary<string, string> sEntries = new Dictionary<string, string>();
    }
}
