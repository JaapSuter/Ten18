using System;
using System.Diagnostics;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.CompilerServices;

namespace Ten18.Interop
{
    /// Based on (copied from) an MSDN blog entry by Rick Byers:
    ///  http://blogs.msdn.com/b/rmbyers/archive/2010/01/30/sample-reflection-emit-code-for-using-exception-filters-from-c.aspx
    public static class ExceptionUtils
    {
        /// <summary>
        /// Execute the body with the specified filter.
        /// </summary>
        /// <param name="body">The code to run inside the "try" block</param>
        /// <param name="filter">Called whenever an exception escapes body, passing the exeption object.  
        /// For exceptions that aren't derived from System.Exception, they'll show up as an instance of RuntimeWrappedException.</param>
        /// <param name="handler">Invoked (with the exception) whenever the filter returns true, causes the exception to be swallowed</param>
        public static void ExecuteWithFilter(Action body, Func<Exception, bool> filter, Action<Exception> handler)
        {
            s_filter(body, filter, handler);
        }

        public static void ExecuteWithFailfast(Action body)
        {
            ExecuteWithFilter(body, (e) =>
            {
                System.Diagnostics.Debugger.Log(10, "ExceptionFilter", "Saw unexpected exception: " + e.ToString());

                // Terminate the process with this fatal error
                if (System.Environment.Version.Major >= 4)
                {
                    // .NET 4 adds a FailFast overload which takes the exception, usefull for getting good watson buckets
                    // This will also cause an attached debugger to stop at the throw point, just as if the exception went unhandled.
                    // Note that this code may be compiled against .NET 2.0 but running in CLR v4, so we want to take advantage of
                    // this API even if it's not available at compile time, so we use a late-bound call.
                    typeof(System.Environment).InvokeMember("FailFast",
                        BindingFlags.Static | BindingFlags.InvokeMethod,
                        null, null, new object[] { "Unexpected Exception", e });
                }
                else
                {
                    // The experience isn't quite as nice in CLR v2 and before (no good watson buckets, debugger shows a 
                    // 'FatalExecutionEngineErrorException' at this point), but still deubggable.
                    System.Environment.FailFast("Exception: " + e.GetType().FullName);
                }

                return false;   // should never be reached
            }, null);
        }

        /// <summary>
        /// Like a normal C# Try/Catch but allows one catch block to catch multiple different types of exceptions.
        /// </summary>
        /// <typeparam name="TExceptionBase">The common base exception type to catch</typeparam>
        /// <param name="body">Code to execute inside the try</param>
        /// <param name="typesToCatch">All exception types to catch (each of which must be derived from or exactly TExceptionBase)</param>
        /// <param name="handler">The catch block to execute when one of the specified exceptions is caught</param>
        public static void TryCatchMultiple<TExceptionBase>(Action body, Type[] typesToCatch, Action<TExceptionBase> handler)
            where TExceptionBase : Exception
        {
            // Verify that every type in typesToCatch is a sub-type of TExceptionBase
#if DEBUG
            foreach (var tc in typesToCatch)
                Debug.Assert(typeof(TExceptionBase).IsAssignableFrom(tc), String.Format("Error: {0} is not a sub-class of {1}",
                    tc.FullName, typeof(TExceptionBase).FullName));
#endif

            ExecuteWithFilter(body, (e) =>
            {
                // If the thrown exception is a sub-type (including the same time) of at least one of the provided types then
                // catch it.
                foreach (var catchType in typesToCatch)
                    if (catchType.IsAssignableFrom(e.GetType()))
                        return true;
                return false;
            }, (e) =>
            {
                handler((TExceptionBase)e);
            });
        }

        /// <summary>
        /// A convenience method for when only the base type of 'Exception' is needed.
        /// </summary>
        public static void TryCatchMultiple(Action body, Type[] typesToCatch, Action<Exception> handler)
        {
            TryCatchMultiple<Exception>(body, typesToCatch, handler);
        }
        /// <summary>
        /// Set to true to write the generated assembly to disk for debugging purposes (eg. to run peverify and
        /// ildasm on in the case of bad codegen).
        /// </summary>
        private static bool k_debug = false;

        /// <summary>
        /// Generate a function which has an EH filter
        /// </summary>
        private static Action<Action, Func<Exception, bool>, Action<Exception>> GenerateFilter()
        {
            // Create a dynamic assembly with reflection emit
            var name = new AssemblyName("DynamicFilter");
            AssemblyBuilder assembly = AppDomain.CurrentDomain.DefineDynamicAssembly(name, k_debug ? AssemblyBuilderAccess.RunAndSave : AssemblyBuilderAccess.Run);
            ModuleBuilder module;
            if (k_debug)
                module = assembly.DefineDynamicModule("DynamicFilter", "DynamicFilter.dll");
            else
                module = assembly.DefineDynamicModule("DynamicFilter");

            // Add an assembly attribute that tells the CLR to wrap non-CLS-compliant exceptions in a RuntimeWrappedException object
            // (so the cast to Exception in the code will always succeed).  C# and VB always do this, C++/CLI doesn't.
            assembly.SetCustomAttribute(new CustomAttributeBuilder(
                typeof(RuntimeCompatibilityAttribute).GetConstructor(new Type[] { }),
                new object[] { },
                new PropertyInfo[] { typeof(RuntimeCompatibilityAttribute).GetProperty("WrapNonExceptionThrows") },
                new object[] { true }));

            // Add an assembly attribute that tells the CLR not to attempt to load PDBs when compiling this assembly 
            assembly.SetCustomAttribute(new CustomAttributeBuilder(
                typeof(DebuggableAttribute).GetConstructor(new Type[] { typeof(DebuggableAttribute.DebuggingModes) }),
                new object[] { DebuggableAttribute.DebuggingModes.IgnoreSymbolStoreSequencePoints }));

            // Create the type and method which will contain the filter
            TypeBuilder type = module.DefineType("Filter", TypeAttributes.Class | TypeAttributes.Public);
            var argTypes = new Type[] { typeof(Action), typeof(Func<Exception, bool>), typeof(Action<Exception>) };
            MethodBuilder meth = type.DefineMethod("InvokeWithFilter", MethodAttributes.Public | MethodAttributes.Static, typeof(void), argTypes);

            var il = meth.GetILGenerator();
            var exLoc = il.DeclareLocal(typeof(Exception));

            // Invoke the body delegate inside the try
            il.BeginExceptionBlock();
            il.Emit(OpCodes.Ldarg_0);
            il.EmitCall(OpCodes.Callvirt, typeof(Action).GetMethod("Invoke"), null);

            // Invoke the filter delegate inside the filter block
            il.BeginExceptFilterBlock();
            il.Emit(OpCodes.Castclass, typeof(Exception));
            il.Emit(OpCodes.Stloc_0);
            il.Emit(OpCodes.Ldarg_1);
            il.Emit(OpCodes.Ldloc_0);
            il.EmitCall(OpCodes.Callvirt, typeof(Func<Exception, bool>).GetMethod("Invoke"), null);

            // Invoke the handler delegate inside the catch block
            il.BeginCatchBlock(null);
            il.Emit(OpCodes.Castclass, typeof(Exception));
            il.Emit(OpCodes.Stloc_0);
            il.Emit(OpCodes.Ldarg_2);
            il.Emit(OpCodes.Ldloc_0);
            il.EmitCall(OpCodes.Callvirt, typeof(Action<Exception>).GetMethod("Invoke"), null);

            il.EndExceptionBlock();
            il.Emit(OpCodes.Ret);

            var bakedType = type.CreateType();
            if (k_debug)
                assembly.Save("DynamicFilter.dll");

            // Construct a delegate to the filter function and return it
            var bakedMeth = bakedType.GetMethod("InvokeWithFilter");
            var del = Delegate.CreateDelegate(typeof(Action<Action, Func<Exception, bool>, Action<Exception>>), bakedMeth);
            return (Action<Action, Func<Exception, bool>, Action<Exception>>)del;
        }

        // Will get generated (with automatic locking) on first use of this class
        private static Action<Action, Func<Exception, bool>, Action<Exception>> s_filter = GenerateFilter();
    }
}
