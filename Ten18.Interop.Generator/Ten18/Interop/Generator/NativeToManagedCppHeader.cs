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
using Microsoft.CSharp;
using Ten18.Interop;
using System.IO;


namespace Ten18.Interop
{
    partial class NativeToManagedCppHeaderTemplate
    {
        internal IEnumerable<string> NameSpaceNames;
        internal Type Type;
        internal MethodInfo[] MethodInfos;

        private string NativeReturnTypeOf(MethodInfo methodInfo)
        {
            return TypeMap.NativeCppNameOf(methodInfo.ReturnType);
        }

        public string NativeParameterListOf(MethodInfo methodInfo)
        {
            return String.Join(", ",
                from pi in methodInfo.GetParameters() select String.Format("{0} {1}", TypeMap.NativeCppNameOf(pi.ParameterType), pi.Name));
        }
    }

    class NativeToManagedCppHeader
    {        
        public static void Generate(IEnumerable<string> nameSpaceNames, Type type, MethodInfo[] methodInfos, string dir)
        {
            var cppHeaderTemplate = new NativeToManagedCppHeaderTemplate()
            {
                 NameSpaceNames = nameSpaceNames,
                 Type = type,
                 MethodInfos = methodInfos,
            };

            dir = Path.Combine(cppHeaderTemplate.NameSpaceNames.StartWith(dir).ToArray());
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            var file = Path.Combine(dir, type.Name + ".Generated.h");
            if (File.Exists(file))
            {
                var backup = Path.Combine(dir, 
                                Path.GetFileNameWithoutExtension(file) + "." + DateTime.Now.ToFileTime().ToString() + Path.GetExtension(file));
                File.Move(file, backup);
            }

            var code = cppHeaderTemplate.TransformText();
            File.WriteAllText(file, code);

            Console.WriteLine("Updated: {0}", file);
        }
    }
}
