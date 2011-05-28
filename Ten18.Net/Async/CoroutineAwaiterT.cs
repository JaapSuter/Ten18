using System;
using System.Disposables;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Ten18.Async
{
    class CoroutineAwaiter : CoroutineAwaiter<Unit>
    {
        public CoroutineAwaiter(CoroutineScheduler scheduler, TaskCompletionSource<Unit> tcs)
            : base(scheduler, tcs)
        { }
    }

    class CoroutineAwaiter<T>
    {
        public CoroutineAwaiter(CoroutineScheduler scheduler, TaskCompletionSource<T> tcs)
        {
            mTaskScheduler = scheduler;
            mTcs = tcs;            
        }

        public bool IsCompleted { get { return mTcs.Task.GetAwaiter().IsCompleted; } }

        public void OnCompleted(Action continuation)
        {
            mTcs.Task.GetAwaiter().OnCompleted(continuation);
        }

        public CoroutineAwaiter<T> GetAwaiter() { return this; }

        public T GetResult() { return mTcs.Task.Result; }

        private CoroutineScheduler mTaskScheduler;
        private TaskCompletionSource<T> mTcs;
    }
}
