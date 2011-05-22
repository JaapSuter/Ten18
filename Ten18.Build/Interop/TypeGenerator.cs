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


namespace Ten18.Interop
{
    abstract class TypeGenerator
    {
        public TypeGenerator(Type type)
        {
            Debug.Assert(!type.IsNested);
            mInteropType = InteropType.Get(type);
            mCppHeaderFile = Path.Combine(Args.Get<string>("WorkingDir"), mInteropType.FullNameInCSharp.Replace(".", "\\") + ".Generated.h");
            if (!Directory.Exists(Path.GetDirectoryName(mCppHeaderFile)))
                Directory.CreateDirectory(Path.GetDirectoryName(mCppHeaderFile));
        }

        public void Generate(ModuleBuilder moduleBuilder)
        {
            GenerateCli(moduleBuilder);
            GenerateCpp();
        }

        protected abstract void GenerateCpp();
        protected abstract void GenerateCli(ModuleBuilder moduleBuilder);
        
        protected readonly InteropType mInteropType;
        protected readonly string mCppHeaderFile;
    }
}
