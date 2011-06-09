using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.CodeDom;
using System.CodeDom.Compiler;
using System.Runtime.CompilerServices;
using Microsoft.CSharp;
using System.IO;
using Mono.Cecil;
using Mono.Cecil.Rocks;
using Mono.Collections.Generic;

namespace Ten18.Interop
{
    static class ParameterDefinitionEx
    {
        public static string FullNameAsCpp(this ParameterDefinition self)
        {
            return self.ParameterType.FullNameAsCpp(!self.IsOut) + " " + self.Name;
        }

        public static string FullNameAsUndecorated(this ParameterDefinition self)
        {
            return self.ParameterType.FullNameAsUndecorated(!self.IsOut);
        }

        public static bool AreSameByType(this Collection<ParameterDefinition> a, Collection<ParameterDefinition> b)
        {
            var count = a.Count;

            if (count != b.Count)
                return false;

            if (count == 0)
                return true;

            for (int i = 0; i < count; i++)
                if (!a[i].ParameterType.IsSame(b[i].ParameterType))
                    return false;

            return true;
        }
    }
}
