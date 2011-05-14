//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: CompletedTask.cs
//
//--------------------------------------------------------------------------

namespace System.Threading.Tasks
{
    /// <summary>Provides access to an already completed task.</summary>
    /// <remarks>A completed task can be useful for using ContinueWith overloads where there aren't StartNew equivalents.</remarks>
    public static class CompletedTask
    {
        /// <summary>Gets a completed Task.</summary>
        public readonly static Task Default = CompletedTask<object>.Default;
    }

    /// <summary>Provides access to an already completed task.</summary>
    /// <remarks>A completed task can be useful for using ContinueWith overloads where there aren't StartNew equivalents.</remarks>
    public class CompletedTask<TResult>
    {
        /// <summary>Initializes a Task to a specific value.</summary>
        public static Task<TResult> WithValue(TResult tr)
        {
            var tcs = new TaskCompletionSource<TResult>();
            tcs.TrySetResult(tr);
            return tcs.Task;
        }

        /// <summary>Initializes the default completed Task.</summary>
        static CompletedTask()
        {
            var tcs = new TaskCompletionSource<TResult>();
            tcs.TrySetResult(default(TResult));
            Default = tcs.Task;
        }

        /// <summary>Gets a completed Task.</summary>
        public readonly static Task<TResult> Default;
    }
}
