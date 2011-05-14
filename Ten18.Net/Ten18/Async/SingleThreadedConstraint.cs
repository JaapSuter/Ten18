using System;
using System.Disposables;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Ten18.Async
{
    struct SingleThreadedConstraint
    {
        public static SingleThreadedConstraint Create()
        {
            return new SingleThreadedConstraint(Thread.CurrentThread.ManagedThreadId);
        }

        public void Verify()
        {
            if (mExclusiveThreadId != Thread.CurrentThread.ManagedThreadId)
                throw new Exception("Method invocation from unexpected thread.");
        }

        private SingleThreadedConstraint(int exclusiveThreadId)
        {
            mExclusiveThreadId = exclusiveThreadId;
        }

        private readonly int mExclusiveThreadId;
    }
}
