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
using System.IO;

namespace Ten18.Interop
{
    class InteropType
    {
        public Type Type { get; private set; }

        public string FullNameInCSharp { get; private set; }
        public string FullNameInCpp { get; private set; }

        public static InteropType Get(Type type)
        {
            InteropType it;
            if (!sCache.TryGetValue(type, out it))
                return new InteropType(type);
            else
                return it;
        }

        private InteropType(Type type)
            : this(type, sCodeProvider.GetTypeOutput(new CodeTypeReference(type)))
        { }

        private InteropType(Type type, string fullNameInCSharp)
            : this(type, fullNameInCSharp, String.Concat("::", fullNameInCSharp.Replace(".", "::")))
        { }

        private InteropType(Type type, string fullNameInCSharp, string fullNameInCpp)
        {
            Debug.Assert(!sCache.ContainsKey(type));
            Type = type;
            FullNameInCSharp = fullNameInCSharp;
            FullNameInCpp = fullNameInCpp;

            sCache.Add(type, this);
        }

        private static Dictionary<Type, InteropType> sCache = new Dictionary<Type, InteropType>();
        private static CodeDomProvider sCodeProvider = CSharpCodeProvider.CreateProvider("C#");

        static InteropType()
        {
            new InteropType(typeof(void), "void", "void");
            new InteropType(typeof(bool), "bool", "bool");
            new InteropType(typeof(float), "float", "float");
            new InteropType(typeof(double), "double", "double");
            new InteropType(typeof(IntPtr), "System.IntPtr", "std::intptr_t");
            new InteropType(typeof(UIntPtr), "System.UIntPtr", "std::uintptr_t");
            new InteropType(typeof(Char), "char", "std::char16_t");
            new InteropType(typeof(SByte), "sbyte", "std::int8_t");
            new InteropType(typeof(Int16), "short", "std::int16_t");
            new InteropType(typeof(Int32), "int", "std::int32_t");
            new InteropType(typeof(Int64), "long", "std::int64_t");
            new InteropType(typeof(Byte), "byte", "std::uint8_t");
            new InteropType(typeof(UInt16), "ushort", "std::uint16_t");
            new InteropType(typeof(UInt32), "uint", "std::uint32_t");
            new InteropType(typeof(UInt64), "ulong", "std::uint64_t");
        }
    }
}
