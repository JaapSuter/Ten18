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
    static class TypeMap
    {
        public static string NativeCppNameOf(Type type)
        {
            return mNativeCppNames[type.FullName];
        }

        struct TypeComparer : IComparer<Type>
        {
            public int Compare(Type x, Type y)
            {
 	            return String.CompareOrdinal(x.FullName, y.FullName);
            }
        }

        private static SortedList<string, string> mNativeCppNames = new SortedList<string, string>
        {
            { typeof(void).FullName, "void" },

            { typeof(IntPtr).FullName, "std::intptr_t" },
            
            { typeof(SByte).FullName, "std::int8_t" },
            { typeof(Int16).FullName, "std::int16_t" },
            { typeof(Int32).FullName, "std::int32_t" },
            { typeof(Int64).FullName, "std::int64_t" },
                           
            { typeof(  Byte).FullName, "std::int8_t" },
            { typeof(UInt16).FullName, "std::int16_t" },
            { typeof(UInt32).FullName, "std::int32_t" },
            { typeof(UInt64).FullName, "std::int64_t" },
            
            { typeof(String).FullName, "std::u16string" },
            { typeof(Char).FullName, "std::char16_t" },
        };
    }
}
