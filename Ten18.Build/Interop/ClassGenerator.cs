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
using SlimMath;


namespace Ten18.Interop
{
    class ClassGenerator : TypeGenerator
    {
        public ClassGenerator(Type type)
            : base(type)
        {
            Debug.Assert(type.IsClass);
            Debug.Assert(type.IsAbstract);
            Debug.Assert(!type.IsSealed);

            mMethodGenerators = type.GetMethods(BindingFlags)
                                    .Where(mi => mi.IsAbstract)
                                    .Select((mi, idx) => new MethodGenerator(mInteropType, mi, idx))
                                    .ToArray();
        }

        protected override void GenerateCpp()
        {
            var cppHeaderTemplate = new CppHeaderTemplate()
            {
                NameSpaceNames = mInteropType.FullNameInCSharp.Split('.'),
                InteropType = mInteropType,
                MethodGenerators = mMethodGenerators,
            };

            var code = cppHeaderTemplate.TransformText();
            File.WriteAllText(mCppHeaderFile, code);

            Console.WriteLine("Updated: {0}", mCppHeaderFile);
        }

        protected override void GenerateCli(ModuleBuilder moduleBuilder)
        {
            var fullName = String.Concat(mInteropType.FullNameInCSharp, "Impl");

            var typeAttributes = TypeAttributes.Class | TypeAttributes.Sealed | TypeAttributes.Public;
            var typeBuilder = moduleBuilder.DefineType(fullName, typeAttributes, mInteropType.Type, Type.EmptyTypes);

            var fieldAttributes = FieldAttributes.Private | FieldAttributes.InitOnly;
            var cppThisPtr = typeBuilder.DefineField("mCppThisPtr", MethodGenerator.CppThisPtrType, fieldAttributes);

            GenerateCliCtor(typeBuilder, cppThisPtr);

            
            mMethodGenerators.Run(mg => mg.GenerateCli(typeBuilder, cppThisPtr));

            GeneratorCliProperties(typeBuilder);

            typeBuilder.CreateType();
        }

        private static void GenerateCliCtor(TypeBuilder typeBuilder, FieldInfo cppThisPtr)
        {
            var nativeNewObj = typeBuilder.DefinePInvokeMethod("New" + typeBuilder.Name, "Ten18.exe",
              MethodAttributes.Private | MethodAttributes.Static | MethodAttributes.PinvokeImpl,
              CallingConventions.Standard, MethodGenerator.CppThisPtrType, Type.EmptyTypes, CallingConvention.StdCall, CharSet.Unicode);
            nativeNewObj.SetImplementationFlags(MethodImplAttributes.PreserveSig | nativeNewObj.GetMethodImplementationFlags());

            var ctorBuilder = typeBuilder.DefineConstructor(MethodAttributes.Public | MethodAttributes.HideBySig, CallingConventions.Standard, Type.EmptyTypes);
            var ctorInBase = typeBuilder.BaseType.GetConstructor(BindingFlags, null, Type.EmptyTypes, null);
            
            var il = ctorBuilder.GetILGenerator();
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, ctorInBase);
            il.Emit(OpCodes.Ldarg_0);
            il.Emit(OpCodes.Call, nativeNewObj);
            il.Emit(OpCodes.Stfld, cppThisPtr);

            il.EmitWriteLine("SizeOf in Bytecode: ");
            var localSizeOf = il.DeclareLocal(typeof(int));
            il.Emit(OpCodes.Sizeof, typeof(Vector3));
            il.Emit(OpCodes.Stloc, localSizeOf.LocalIndex);
            il.EmitWriteLine(localSizeOf);
            
            il.Emit(OpCodes.Ret);
        }

        private void GeneratorCliProperties(TypeBuilder typeBuilder)
        {
            var pgs = from propertyInfo in mInteropType.Type.GetProperties(BindingFlags)
                      let getMethodBase = propertyInfo.GetGetMethod(nonPublic: true)
                      let setMethodBase = propertyInfo.GetSetMethod(nonPublic: true)
                      where getMethodBase != null && getMethodBase.IsAbstract
                         || setMethodBase != null && setMethodBase.IsAbstract
                      let getMethodImpl = mMethodGenerators.SingleOrDefault(mg => mg.MethodInfo == getMethodBase)
                      let setMethodImpl = mMethodGenerators.SingleOrDefault(mg => mg.MethodInfo == setMethodBase)
                      where getMethodImpl != null 
                         || setMethodImpl != null
                      select new
                      {
                          PropertyInfo = propertyInfo,
                          SetMethod = setMethodImpl,
                          GetMethod = getMethodImpl,
                      };

            foreach (var pg in pgs)
            {
                var propertyBuilder = typeBuilder.DefineProperty(pg.PropertyInfo.Name, pg.PropertyInfo.Attributes, pg.PropertyInfo.PropertyType, Type.EmptyTypes);

                if (pg.GetMethod != null)
                    propertyBuilder.SetGetMethod(pg.GetMethod.MethodBuilder);

                if (pg.SetMethod != null)
                    propertyBuilder.SetSetMethod(pg.SetMethod.MethodBuilder);
            }
        }

        private readonly MethodGenerator[] mMethodGenerators;

        private static readonly BindingFlags BindingFlags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;
    }
}
