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
using Ten18.Interop;
using System.Linq.Expressions;

namespace Ten18.Interop
{
    unsafe class Sandbox
    {
        public Sandbox(IntPtr rawNativePtr)
        {
            mRawNativePtr = rawNativePtr.ToPointer();            
        }

        public void Action(int i)
        {
            NativeFunc(mRawNativePtr, i);
        }
        
        [DllImport("Ten18.exe", CallingConvention = CallingConvention.StdCall)]
        private extern static void NativeFunc(void* ptr, int i);

        private readonly void* mRawNativePtr;
    }
}
