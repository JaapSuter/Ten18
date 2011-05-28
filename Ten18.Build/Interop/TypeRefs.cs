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

namespace Ten18.Interop
{
    static class TypeRefs
    {
        public static TypeReference Object { get; private set; }
        public static TypeReference IntPtr { get; private set; }
        public static TypeReference Void { get; private set; }
        public static TypeReference VoidStar { get; private set; }
        public static TypeReference Char { get; private set; }
        
        public static TypeDefinition Boolean { get; private set; }
        public static TypeDefinition String { get; private set; }
        public static TypeDefinition NativeFactory { get; private set; }

        public static PatchTableTemplate PatchTable { get; private set; }
        
        public static int SizeOfVTableSlot { get; private set; }
        public static int SizeOfRegister { get; private set; }
        
        public static void Initialize(ModuleDefinition moduleDef)
        {
            SizeOfRegister = System.IntPtr.Size;
            SizeOfVTableSlot = System.IntPtr.Size;
            
            var typeSystem = moduleDef.TypeSystem;
            
            NativeFactory = Register(moduleDef, "Ten18.Interop.NativeFactory").Resolve();
            String = Register(moduleDef, typeSystem.String, "const wchar_t*").Resolve();
            Void = Register(moduleDef, typeSystem.Void, "void");
            VoidStar = Register(moduleDef, typeSystem.Void.MakePointerType(), "void*");
            Object = Register(moduleDef, typeSystem.Object, "object");
            IntPtr = Register(moduleDef, typeSystem.IntPtr, "std::intptr_t");
            Char = Register(moduleDef, typeSystem.Char, "wchar_t");
            Boolean = Register(moduleDef, typeSystem.Boolean, "::Ten18::Interop::Boolean").Resolve();

            Register(moduleDef, typeSystem.Single, "float");
            Register(moduleDef, typeSystem.Double, "double");            
            Register(moduleDef, typeSystem.UIntPtr, "std::uintptr_t");            
            Register(moduleDef, typeSystem.SByte, "std::int8_t");
            Register(moduleDef, typeSystem.Int16, "std::int16_t");
            Register(moduleDef, typeSystem.Int32, "std::int32_t");
            Register(moduleDef, typeSystem.Int64, "std::int64_t");
            Register(moduleDef, typeSystem.Byte, "std::uint8_t");
            Register(moduleDef, typeSystem.UInt16, "std::uint16_t");
            Register(moduleDef, typeSystem.UInt32, "std::uint32_t");
            Register(moduleDef, typeSystem.UInt64, "std::uint64_t");

            var vector2 = Register(moduleDef, typeof(SlimMath.Vector2), "XMFLOAT2");
            var vector3 = Register(moduleDef, typeof(SlimMath.Vector3), "XMFLOAT3");
            var vector4 = Register(moduleDef, typeof(SlimMath.Vector4), "XMFLOAT4");
            var matrix =  Register(moduleDef, typeof(SlimMath.Matrix), "XMFLOAT4X4");

            PatchTable = new PatchTableTemplate();
        }

        public static bool CanBePassedAroundInRegister(this TypeReference typeRef)
        {
            Debug.Assert(!typeRef.IsByReference);
            Debug.Assert(!typeRef.IsFunctionPointer);

            if (typeRef.IsPrimitive || typeRef.IsArray || !typeRef.IsValueType || typeRef.IsPointer)
                return true;

            var typeDef = typeRef.Resolve();
            if (typeDef.Fields.Count == 0)
                return true;

            if (typeDef.Fields.Count == 1)
                return typeDef.Fields[0].FieldType.CanBePassedAroundInRegister();

            return false;
        }
        
        public static string FullNameInCpp(this TypeReference typeRef)
        {
            return Cache(typeRef).FullNameInCpp;
        }

        private class CppTypeName
        {
            public TypeReference TypeRef;
            public string FullNameInCpp;
        }

        private class CppTypeNames : KeyedCollection<string, CppTypeName>
        {
            protected override string GetKeyForItem(CppTypeName typeName)
            {
                return typeName.TypeRef.FullName;
            }
        }

        private static CppTypeName Cache(TypeReference typeRef, string cppTypeName = null)
        {
            CppTypeName ret = null;
            if (!sCppTypeNames.Contains(typeRef.FullName))
            {
                ret = new CppTypeName() { TypeRef = typeRef, FullNameInCpp = cppTypeName ?? ("::" + typeRef.FullName.Replace(".", "::")), };
                sCppTypeNames.Add(ret);
            }
            else ret = sCppTypeNames[typeRef.FullName];

            return ret;
        }

        private static TypeReference Register(ModuleDefinition moduleDef, string csTypeName, string cppTypeName = null)
        {
            return Cache(moduleDef.Import(moduleDef.GetType(csTypeName)), cppTypeName).TypeRef;
        }

        private static TypeReference Register(ModuleDefinition moduleDef, TypeReference typeRef, string cppTypeName = null)
        {
            return Cache(moduleDef.Import(typeRef), cppTypeName).TypeRef;
        }

        private static TypeReference Register(ModuleDefinition moduleDef, Type type, string cppTypeName = null)
        {
            return Cache(moduleDef.Import(type), cppTypeName).TypeRef;
        }

        private static CppTypeNames sCppTypeNames = new CppTypeNames();
    }
}
