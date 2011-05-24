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
    partial class CppHeaderTemplate
    {
        internal InteropType InteropType;
        internal MethodGenerator[] MethodGenerators;
    }
}
