using System;
using System.Disposables;
using System.Linq;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace Ten18.Async
{
    struct SingleThreadConstraint
    {
        public static SingleThreadConstraint Create()
        {
            return new SingleThreadConstraint(Thread.CurrentThread.ManagedThreadId);
        }

        public void Verify()
        {
            if (mExclusiveThreadId != Thread.CurrentThread.ManagedThreadId)
                throw new Exception("Method invocation from unexpected thread.");
        }

        private SingleThreadConstraint(int exclusiveThreadId)
        {
            mExclusiveThreadId = exclusiveThreadId;
        }

        private readonly int mExclusiveThreadId;
    }
}
