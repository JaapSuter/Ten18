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

namespace Ten18.Interop
{
    class InteropType
    {
        // Would've liked to use IntPtr, but its IL behaviour annoyed me a bit because it's a compound struct, so 
        // I dropped down to a plain old 32 bit integer, which 'll work on x86 just fine. It's not like I'll ever
        // get to a x64 or ARM port anyway... ha!
        public static TypeReference CppThisPtrTypeRef { get; private set; }
        public static TypeReference VoidTypeRef { get; private set; }
        public static TypeReference ObjectTypeRef { get; private set; }        
        public static int VTableSlotSize { get; private set; }
        public static int SizeOfRegisterReturn { get; private set; }
        public static TypeDefinition NativeTypeFactoryDef { get; private set; }
        public static FieldDefinition NativeTypeFactoryInstanceDef { get; private set; }

        public static void Initialize(ModuleDefinition moduleDef)
        {
            VoidTypeRef = moduleDef.TypeSystem.Void;
            ObjectTypeRef = moduleDef.TypeSystem.Object;
            CppThisPtrTypeRef = moduleDef.TypeSystem.UInt32;
            VTableSlotSize = sizeof(UInt32);
            SizeOfRegisterReturn = sizeof(UInt32);

            DefineNativeTypeFactory(moduleDef);

            new InteropType(moduleDef.TypeSystem.Void, "void");
            new InteropType(moduleDef.TypeSystem.Boolean, "bool");
            new InteropType(moduleDef.TypeSystem.Single, "float");
            new InteropType(moduleDef.TypeSystem.Double, "double");
            new InteropType(moduleDef.TypeSystem.IntPtr, "std::intptr_t");
            new InteropType(moduleDef.TypeSystem.UIntPtr, "std::uintptr_t");
            new InteropType(moduleDef.TypeSystem.Char, "std::char16_t");
            new InteropType(moduleDef.TypeSystem.SByte, "std::int8_t");
            new InteropType(moduleDef.TypeSystem.Int16, "std::int16_t");
            new InteropType(moduleDef.TypeSystem.Int32, "std::int32_t");
            new InteropType(moduleDef.TypeSystem.Int64, "std::int64_t");
            new InteropType(moduleDef.TypeSystem.Byte, "std::uint8_t");
            new InteropType(moduleDef.TypeSystem.UInt16, "std::uint16_t");
            new InteropType(moduleDef.TypeSystem.UInt32, "std::uint32_t");
            new InteropType(moduleDef.TypeSystem.UInt64, "std::uint64_t");
        }

        public TypeReference TypeRef { get; private set; }

        public string FullNameInCSharp { get; private set; }
        public string FullNameInCpp { get; private set; }

        public static InteropType Get(TypeReference typeRef)
        {
            if (sCache.Contains(typeRef))
                return sCache[typeRef];
            else
                return new InteropType(typeRef);
        }

        public static string GetFullNameInCpp(TypeReference typeRef)
        {
            return Get(typeRef).FullNameInCpp;
        }

        private InteropType(TypeReference typeRef)
            : this(typeRef, String.Concat("::", typeRef.FullName.Replace(".", "::")))
        { }

        private InteropType(TypeReference typeRef, string fullNameInCpp)
        {
            Debug.Assert(!sCache.Contains(typeRef));
            TypeRef = typeRef;
            FullNameInCSharp = typeRef.FullName;
            FullNameInCpp = fullNameInCpp;

            sCache.Add(this);
        }

        private static void DefineNativeTypeFactory(ModuleDefinition moduleDef)
        {
            NativeTypeFactoryDef = new TypeDefinition("Ten18.Interop", "NativeTypeFactory", TypeAttributes.Class | TypeAttributes.Abstract | TypeAttributes.Public);
            NativeTypeFactoryInstanceDef = new FieldDefinition("Instance", FieldAttributes.Public | FieldAttributes.Static, NativeTypeFactoryDef);
            NativeTypeFactoryDef.Fields.Add(NativeTypeFactoryInstanceDef);            
        }

        public static void CompleteNativeTypeFactory(ModuleDefinition moduleDef)
        {            
            TypeGenerator.Create(NativeTypeFactoryDef).Generate();
            moduleDef.Types.Add(NativeTypeFactoryDef);
        }

        private class Cache : KeyedCollection<TypeReference, InteropType>
        {
            protected override TypeReference GetKeyForItem(InteropType item)
            {
                return item.TypeRef;
            }
        }

        private static Cache sCache = new Cache();
    }
}
