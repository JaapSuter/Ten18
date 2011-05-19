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

namespace Ten18
{
    static class BinToHeader
    {
        public static void Convert(byte[] bin, TextWriter header, string name)
        {
            
            var sb = new StringBuilder(bin.Length * OverheadPerCharIsh + OverheadConstantGuess);
            sb.AppendFormat("static const int {0}Size = {1};", name, bin.Length);
            sb.AppendLine();
            sb.AppendFormat("static __declspec(align(16)) const char {0}[{0}Size] = {{", name);
            sb.AppendLine();            
            
            foreach (var line in Linify(bin))
                sb.AppendLine(line);

            sb.AppendLine("};");
            sb.AppendLine();
                        
            header.Write(sb.ToString());
        }

        private static IEnumerable<string> Linify(byte[] bin)
        {
            var indent = "    ";
            var sb = new StringBuilder(indent, LineLengthIsh);
            foreach (var b in bin)
            {
                sb.AppendFormat("{0}, ", (sbyte)b);

                if (LineLengthIsh < sb.Length)
                {
                    yield return sb.ToString();
                    sb.Clear();
                    sb.Append(indent);
                }
            }
        }

        private const int LineLengthIsh = 87;
        private const int OverheadPerCharIsh = 4;
        private const int OverheadConstantGuess = 1024;
    }
}