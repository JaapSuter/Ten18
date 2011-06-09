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
    static class TypeReferenceEx
    {
        public static bool IsSame(this TypeReference self, TypeReference other)
        {
            return self.FullName == other.FullName
                && self.Resolve().Module.FullyQualifiedName == other.Resolve().Module.FullyQualifiedName;
        }

        public static bool CanBePassedAroundInRegister(this TypeReference typeRef)
        {
            Debug.Assert(!typeRef.IsByReference);
            Debug.Assert(!typeRef.IsFunctionPointer);

            if (typeRef.IsPrimitive || typeRef.IsArray || !typeRef.IsValueType || typeRef.IsPointer || typeRef.IsSame(Globals.Void))
                return true;

            var typeDef = typeRef.Resolve();
            if (typeDef.Fields.Count == 0)
                return true;

            if (typeDef.Fields.Count == 1)
                return typeDef.Fields[0].FieldType.CanBePassedAroundInRegister();

            return false;
        }
        
        public static string FullNameAsCpp(this TypeReference self, bool isConst)
        {
            var constFix = isConst ? "const " : "";
            switch (self.MetadataType)
            {
                case MetadataType.Boolean:
                    return "bool";
                case MetadataType.ByReference:
                    return constFix + FullNameAsCpp(self.GetElementType(), isConst: false) + "&";
                case MetadataType.Byte:
                    return "std::uint8_t";
                case MetadataType.Char:
                    return "wchar_t";
                case MetadataType.Double:
                    return "double";
                case MetadataType.Int16:
                    return "std::int16_t";
                case MetadataType.Int32:
                    return "std::int32_t";
                case MetadataType.Int64:
                    return "std::int64_t";
                case MetadataType.IntPtr:
                    return "std::intptr_t";
                case MetadataType.Pointer:
                    return constFix + FullNameAsCpp(self.GetElementType(), isConst: false) + "*";
                case MetadataType.SByte:
                    return "std::int8_t";
                case MetadataType.Single:
                    return "float";
                case MetadataType.String:
                    return "const wchar_t*";
                case MetadataType.UInt16:
                    return "std::uint16_t";
                case MetadataType.UInt32:
                    return "std::uint32_t";
                case MetadataType.UInt64:
                    return "std::uint64_t";
                case MetadataType.UIntPtr:
                    return "std::uintptr_t";
                case MetadataType.ValueType:
                    if (self.IsSame(self.Module.Import(typeof(SlimMath.Vector2)))) return "_XMFLOAT2";
                    if (self.IsSame(self.Module.Import(typeof(SlimMath.Vector3)))) return "_XMFLOAT3";
                    if (self.IsSame(self.Module.Import(typeof(SlimMath.Vector4)))) return "_XMFLOAT4";
                    if (self.IsSame(self.Module.Import(typeof(SlimMath.Matrix)))) return "_XMFLOAT4X4";

                    return self.FullName.Replace(".", "::");
                case MetadataType.Void:
                    return "void";
                default:
                    var error = "[Unhandled C++ Type For: " + self.FullName + "]";
                    Debug.Fail(error);
                    return error;
            }
        }

        public static string FullNameAsUndecorated(this TypeReference self, bool isConst)
        {
            var constFix = isConst ? " const " : " ";
            switch (self.MetadataType)
            {
                case MetadataType.ByReference:
                    return FullNameAsUndecorated(self.GetElementType(), isConst: false) + constFix + "&";
                case MetadataType.Pointer:
                    return FullNameAsUndecorated(self.GetElementType(), isConst: false) + constFix + "*";
                case MetadataType.String:
                    return "wchar_t const *";
                default:
                    return FullNameAsCpp(self.GetElementType(), isConst);
            }            
        }
    }
}
