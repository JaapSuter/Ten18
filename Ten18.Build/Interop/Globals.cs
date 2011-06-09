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
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
using Microsoft.CSharp;
using System.IO;
using Mono.Cecil;
using Mono.Cecil.Rocks;
using System.Security;

namespace Ten18.Interop
{
    static class Globals
    {
        public const string NameOfCppThisPtrParameter = "cppThisPtr";
        
        public static ModuleReference PInvokeModuleRef { get; private set; }

        public static MethodReference SupressUnmanagedCodeSecurityCtor { get; private set; }
        
        public static TypeReference Object { get; private set; }
        public static TypeReference Boolean { get; private set; }
        public static TypeReference IntPtr { get; private set; }
        public static TypeReference Void { get; private set; }
        public static TypeReference String { get; private set; }
        public static TypeReference VoidStar { get; private set; }
        public static TypeReference Char { get; private set; }
                
        public static int SizeOfVTableSlot { get; private set; }
        public static int SizeOfRegister { get; private set; }
        
        public static void Initialize(ModuleDefinition moduleDef)
        {
            SizeOfRegister = System.IntPtr.Size;
            SizeOfVTableSlot = System.IntPtr.Size;

            PInvokeModuleRef = new ModuleReference("Ten18.exe");
            moduleDef.ModuleReferences.Add(PInvokeModuleRef);

            SupressUnmanagedCodeSecurityCtor = moduleDef.Import(typeof(SuppressUnmanagedCodeSecurityAttribute).GetConstructor(Type.EmptyTypes));

            String = moduleDef.Import(typeof(string));
            Boolean = moduleDef.Import(typeof(bool));
            Void = moduleDef.Import(typeof(void));
            Object = moduleDef.Import(typeof(object));
            IntPtr = moduleDef.Import(typeof(IntPtr));
            Char = moduleDef.Import(typeof(Char));
            VoidStar = moduleDef.Import(typeof(void).MakePointerType());
        }
    }
}
