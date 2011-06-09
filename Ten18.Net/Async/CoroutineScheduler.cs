using System;
using System.Concurrency;
using System.Disposables;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Ten18.Async
{
    class CoroutineScheduler : TaskScheduler
    {
        public TaskFactory TaskFactory { get; private set; }

        public CoroutineScheduler()
        {
            if (SynchronizationContext.Current != null)
                throw new InvalidProgramException("SynchronizationContext.Current should not be set already.");            
            mSynchronizationContext = new CoroutineSynchronizationContext(this);
            SynchronizationContext.SetSynchronizationContext(mSynchronizationContext);

            TaskFactory = new TaskFactory(this);
        }

        public override int MaximumConcurrencyLevel { get { Verify(); return 1; } }
        
        protected override void QueueTask(Task task)
        {
            Verify();
            if (mIsExecutingTasks)
                if (TryExecuteTaskInline(task, false))
                    return;

            mQueued.Enqueue(task);
        }

        public void AfterNextTickStart(Task task)
        {
            Debug.Assert(task.Status == TaskStatus.Created);
            mAfterNextTick.Enqueue(task);
        }

        public CoroutineAwaiter Yield()
        {
            return new CoroutineAwaiter(this);
        }

        public void Tick()
        {
            Verify();
            Debug.Assert(mTryAgain.IsEmpty());

            while (!mAfterNextTick.IsEmpty())
                mAfterNextTick.Dequeue().Start(this);

            using (Disposable.Create(() => mIsExecutingTasks = false))
            {
                mIsExecutingTasks = true;

                for (bool didSomeWork = true; didSomeWork; )
                {
                    didSomeWork = false;
                    while (mQueued.Count > 0)
                    {
                        var task = mQueued.Dequeue();
                        var ran = TryExecuteTaskInline(task, true);

                        if (ran)
                            didSomeWork = true;
                        else
                            mTryAgain.Enqueue(task);
                    }

                    Util.Swap(ref mTryAgain, ref mQueued);
                }
            }
        }

        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
            Verify();
            
            if (!mIsExecutingTasks)
                return false;

            if (taskWasPreviouslyQueued)
                TryDequeue(task);

            return base.TryExecuteTask(task);
        }

        protected override bool TryDequeue(Task task)
        {
            Verify();
            Debug.Assert(!mQueued.Contains(task));
            Debug.Assert(!mTryAgain.Contains(task));            
            return true;
        }

        protected override IEnumerable<Task> GetScheduledTasks()
        {
            return EnumerableEx.Concat(mQueued, mTryAgain).ToArray();
        }

        private void Verify()
        {
            mSingleThreadedConstraint.Verify();
            Debug.Assert(SynchronizationContext.Current == mSynchronizationContext);
            Debug.Assert(!GetScheduledTasks().HasDuplicates());
        }

        private bool mIsExecutingTasks;
        private Queue<Task> mQueued = new Queue<Task>();
        private Queue<Task> mTryAgain = new Queue<Task>();
        private Queue<Task> mAfterNextTick = new Queue<Task>();
        private readonly SingleThreadedConstraint mSingleThreadedConstraint = SingleThreadedConstraint.Create();
        private readonly CoroutineSynchronizationContext mSynchronizationContext;
    }
}
