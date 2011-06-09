using System.Collections.Generic;
using System.Linq;
using System;
using System.IO;
using Ten18.Build;
using System.Text.RegularExpressions;
using System.Diagnostics;

namespace Ten18.Interop
{
    static class ExportTable
    {
        public static int GetKnownOrRegisterForNewOrdinal(MethodGenerator methodGenerator)
        {
            int ordinal;
            var fullNameAsUndecorated = methodGenerator.FullNameAsUndecorated();
            if (sOrdinals.TryGetValue(fullNameAsUndecorated, out ordinal))
                return ordinal;

            ordinal = sOrdinals.Count + 1;
            Debug.Assert(sOrdinals.IsEmpty() || sOrdinals.Values.Max() < ordinal);
            sOrdinals[fullNameAsUndecorated] = ordinal;

            return ordinal;
        }

        public static void Verify()
        {
            var lookup = GetNativeExports();

            foreach (var export in sOrdinals.Keys)
            {
                int ordinal;
                if (!lookup.TryGetValue(export, out ordinal))
                {
                    Console.WriteLine("Missing native export [{0}]: {1}", sOrdinals[export], export);
                }
                else if (ordinal != sOrdinals[export])
                {
                    Console.WriteLine("Managed import ordinal [{0}] doesn't match native export ordinal [{1}]: {2}", sOrdinals[export], ordinal, export);
                }
                else
                {
                    Console.WriteLine("Managed export verified with native ordinal ok: [{0}]: {1}", sOrdinals[export], export);
                }
            }
            
            var lines = from kvp in lookup
                        orderby kvp.Value
                        select kvp.Key;

            File.WriteAllLines(sExportsFile, lines);
        }

        private static IDictionary<string, int> GetNativeExports()
        {
            var exe = Path.Combine(Paths.WorkingDir.Replace("obj", "bin"), "Ten18.exe");

            if (!File.Exists(exe))
                return new Dictionary<string, int>();

            var dped = Tool.Run(Paths.DumpBin, Paths.MsPdbDllDir, "/EXPORTS {0}", exe);

            var regex = new Regex(@"\s+(?<Ordinal>[a-fA-F0-9]+)\s+\w+\s+\w+\s+(?<DecoratedName>\S+) \= .*", RegexOptions.ExplicitCapture);
            var exports = from line in dped.Split('\n')
                          let match = regex.Match(line)
                          where match.Success
                          select new
                          { 
                              Ordinal = int.Parse(match.Groups["Ordinal"].Value),
                              DecoratedName = match.Groups["DecoratedName"].Value
                          };

            return exports.ToDictionary(export => Undecorate(export.DecoratedName), export => export.Ordinal);
        }

        private static string Undecorate(string decoratedName)
        {
            // UndName Flags:
            //     0x0001  Remove leading underscores from Microsoft extended keywords
            //     0x0002  Disable expansion of Microsoft extended keywords
            //     0x0004  Disable expansion of return type for primary declaration
            //     0x0008  Disable expansion of the declaration model
            //     0x0010  Disable expansion of the declaration language specifier
            //     0x0060  Disable all modifiers on the 'this' type
            //     0x0080  Disable expansion of access specifiers for members
            //     0x0100  Disable expansion of 'throw-signatures' for functions and pointers to
            //             functions
            //     0x0200  Disable expansion of 'static' or 'virtual'ness of members
            //     0x0400  Disable expansion of Microsoft model for UDT returns
            //     0x0800  Undecorate 32-bit decorated names
            //     0x1000  Crack only the name for primary declaration
            //             return just [scope::]name.  Does expand template params
            //     0x2000  Input is just a type encoding; compose an abstract declarator
            //     0x8000  Disable enum/class/struct/union prefix
            //     0x20000 Disable expansion of __ptr64 keyword
            var undecorationFlags = 0x8000;
            var undecoratedPrefix = "is :- ";
            return Tool.Run(Paths.UndName, Paths.MsPdbDllDir, "{0} {1}", undecorationFlags.ToString(), decoratedName)
                        .Split('\n')
                        .First(l => l.StartsWith(undecoratedPrefix))
                        .Replace(undecoratedPrefix, "")
                        .Trim('\"')
                        .Replace("(void)", "()");
        }

        private static readonly string sExportsFile = Path.Combine(Paths.WorkingDir, "Ten18.Interop.Native.Exports.txt");
        private static readonly IDictionary<string, int> sOrdinals = Initialize();

        private static IDictionary<string, int> Initialize()
        {
            if (!File.Exists(sExportsFile))
                return new Dictionary<string, int>();

            const int ordinalsAreOneBasedNotZero = 1;
            return File.ReadLines(sExportsFile)
                       .Select((line, idx) => Tuple.Create(line, idx + ordinalsAreOneBasedNotZero))
                       .ToDictionary(t => t.Item1, t => t.Item2);
        }
    }
}
