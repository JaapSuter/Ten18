using System;
using System.Disposables;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Ten18.Async
{
    class CoroutineAwaiter<T>
    {
        public CoroutineAwaiter(Heart scheduler)
        {
            mTaskScheduler = scheduler;
        }

        public bool IsCompleted { get { return false; } }

        public void OnCompleted(Func<T> continuation)
        {
            mTask = new Task<T>(continuation);
            mTaskScheduler.AfterNextTickStart(mTask);
        }

        public CoroutineAwaiter<T> GetAwaiter() { return this; }

        public T GetResult() { Debug.Assert(mTask.IsCompleted); return mTask.Result; }

        private Heart mTaskScheduler;
        private Task<T> mTask;
    }

    class CoroutineAwaiter
    {
        public CoroutineAwaiter(Heart scheduler)
        {
            mTaskScheduler = scheduler;
        }

        public bool IsCompleted { get { return false; } }

        public void OnCompleted(Action continuation)
        {
            mTask = new Task(continuation);
            mTaskScheduler.AfterNextTickStart(mTask);
        }

        public CoroutineAwaiter GetAwaiter() { return this; }

        public void GetResult() {}

        private Heart mTaskScheduler;
        private Task mTask;
    }    
}
