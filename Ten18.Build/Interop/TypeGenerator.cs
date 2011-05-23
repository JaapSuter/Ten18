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
using System.Linq.Expressions;
using Mono.Cecil;


namespace Ten18.Interop
{
    abstract class TypeGenerator
    {
        public InteropType InteropType { get; private set; }
        public TypeDefinition TypeDef { get; private set; }

        protected string CppHeaderFile { get; private set; }

        public static TypeGenerator Create(TypeDefinition typeDef)
        {
            if (typeDef.IsClass && !typeDef.IsValueType && typeDef.IsAbstract)
                return new ClassGenerator(typeDef);
            else if (typeDef.IsEnum)
                return null;
            else
                return null;
        }

        public TypeGenerator(TypeDefinition typeDef)
        {
            Debug.Assert(!typeDef.IsNested);
            InteropType = InteropType.Get(typeDef);
            TypeDef = typeDef;
            CppHeaderFile = Path.Combine(Args.Get<string>("WorkingDir"), InteropType.FullNameInCSharp.Replace(".", "\\") + ".Generated.h");
            if (!Directory.Exists(Path.GetDirectoryName(CppHeaderFile)))
                Directory.CreateDirectory(Path.GetDirectoryName(CppHeaderFile));
        }

        public void Generate()
        {
            GenerateCli();
            GenerateCpp();
        }

        protected abstract void GenerateCpp();
        protected abstract void GenerateCli();
    }
}
