using System;
using System.Linq;
using System.Diagnostics;
using Mono.Cecil;

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
                    return "unsigned char";
                case MetadataType.Char:
                    return "wchar_t";
                case MetadataType.Double:
                    return "double";
                case MetadataType.Int16:
                    return "short";
                case MetadataType.Int32:
                    return "int";
                case MetadataType.Int64:
                    return "__int64";
                case MetadataType.IntPtr:
                    return "std::intptr_t";
                case MetadataType.Pointer:
                    return constFix + FullNameAsCpp(self.GetElementType(), isConst: false) + "*";
                case MetadataType.SByte:
                    return "char";
                case MetadataType.Single:
                    return "float";
                case MetadataType.String:
                    return "const wchar_t*";
                case MetadataType.UInt16:
                    return "unsigned short";
                case MetadataType.UInt32:
                    return "unsigned int";
                case MetadataType.UInt64:
                    return "unsigned __int64";
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
                    return self.FullName.Replace(".", "::") + "*";
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
                case MetadataType.IntPtr:
                    return "int";
                case MetadataType.UIntPtr:
                    return "unsigned int";
                case MetadataType.Class:
                    return self.FullName.Replace(".", "::") + " *";
                default:
                    return FullNameAsCpp(self.GetElementType(), isConst);
            }            
        }
    }
}
