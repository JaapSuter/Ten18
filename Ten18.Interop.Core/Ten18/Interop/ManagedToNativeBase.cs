using System;
using System.Linq;
using System.Linq.Expressions;
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
    public abstract class ManagedToNativeBase
    {
        protected ManagedToNativeBase(object obj)
        {
            mObj = obj;
            mGCHandle = GCHandle.Alloc(this, GCHandleType.Normal);
            mNativeHandle = GCHandle.ToIntPtr(mGCHandle);
            
            // Todo, Jaap Suter, April 2011, need to call mGCHandle.Free at some point, to IDisposable or not, lifetime questions deferred.... :-)
                        
            // if (sMethodMap == null)
                // Initialize(obj.GetType());
        }

        private static void Generate(TypeBuilder typeBuilder)
        {
        }
    
        private static void Initialize(Type type)
        {
            /*
            var bindingFlags = BindingFlags.DeclaredOnly | BindingFlags.Instance | BindingFlags.Public;

            var methodMap = from methodInfo in type.GetMethods(bindingFlags)
                            let managedDelegateType = Delegate.CreateDelegate(type, methodInfo).GetType()
                            let managedDelegate = Delegate.CreateDelegate(managedDelegateType, null, methodInfo)                            
                            select new MethodToDelegateToNativeFuncPtr
                            {
                                MethodInfo = methodInfo,
                                ManagedDelegate = managedDelegate,
                                NativeFuncPtr = Marshal.GetFunctionPointerForDelegate(managedDelegate)
                             };

            sMethodMap = methodMap.ToList();
            */
        }
        
        /*
        private struct MethodToDelegateToNativeFuncPtr
        {
            public MethodInfo MethodInfo;
            public Delegate ManagedDelegate;
            public IntPtr NativeFuncPtr;
        }
        */

        private GCHandle mGCHandle;
        private IntPtr mNativeHandle;

        protected object mObj;

        // private static List<MethodToDelegateToNativeFuncPtr> sMethodMap;
    }
}
