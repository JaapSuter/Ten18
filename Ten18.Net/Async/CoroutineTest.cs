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
    public static class CoroutineTest
    {
        public static void Test()
        {
            mVal = 0;

            TestSingleTask();
            TestMultipleTasks();
            TestContinuation();
            TestWhenAll();            
            TestAsyncAwait();
            TestAsyncAwaitWithReturnValue();
            TestAsyncAwaitOnAlreadyCompleted();
        }

        private static void TestSingleTask()
        {
            Debug.Assert(mVal == 0);

            var task = sCoroFactory.StartNew(() => { Debug.Assert(0 == mVal); mVal = 1; });

            Debug.Assert(!task.IsCompleted);
            Debug.Assert(mVal == 0);

            sCoroScheduler.Tick();

            Debug.Assert(task.IsCompleted);
            Debug.Assert(mVal == 1);
        }

        private static void TestMultipleTasks()
        {
            mVal = 0;
            Debug.Assert(mVal == 0);

            var action = new Action(() => { ++mVal; });
            
            const int num = 5;
            var tasks = new List<Task>();
            for (int i = 0; i < num; ++i)
            {
                tasks.Add(sCoroFactory.StartNew(action));
                Debug.Assert(mVal == 0);
            }

            Debug.Assert(tasks.All(t => !t.IsCompleted));
            Debug.Assert(mVal == 0);

            sCoroScheduler.Tick();

            Debug.Assert(tasks.All(t => t.IsCompleted)); 
            Debug.Assert(mVal == num);
        }

        private static void TestContinuation()
        {
            mVal = 0;
            Debug.Assert(mVal == 0);

            var task = sCoroFactory.StartNew(() => { Debug.Assert(0 == mVal); mVal = 1; });
            Debug.Assert(mVal == 0);

            var continuation = task.ContinueWith(t =>
            { 
                Debug.Assert(t.IsCompleted);
                Debug.Assert(task.IsCompleted);
                Debug.Assert(mVal == 1);
                mVal = 2;                
            }, sCoroScheduler);

            Debug.Assert(mVal == 0);
            Debug.Assert(!task.IsCompleted);
            Debug.Assert(!continuation.IsCompleted);

            sCoroScheduler.Tick();

            Debug.Assert(task.IsCompleted);
            Debug.Assert(continuation.IsCompleted);
            Debug.Assert(mVal == 2);
        }

        private static void TestWhenAll()
        {
            mVal = 0;
            Debug.Assert(mVal == 0);

            var action = new Action(() => { sConstraint.Verify(); ++mVal; });

            const int num = 5;
            var tasks = new List<Task>();
            for (int i = 0; i < num; ++i)
            {
                tasks.Add(sCoroFactory.StartNew(action));
                Debug.Assert(mVal == 0);
            }

            Debug.Assert(tasks.All(t => !t.IsCompleted));
            Debug.Assert(mVal == 0);

            var whenAll = sCoroFactory.ContinueWhenAll(tasks.ToArray(), _ => action(), sCoroFactory.CancellationToken, sCoroFactory.ContinuationOptions, sCoroScheduler);
            
            Debug.Assert(!whenAll.IsCompleted);
            Debug.Assert(mVal == 0);

            sCoroScheduler.Tick();

            Debug.Assert(tasks.All(t => t.IsCompleted));
            Debug.Assert(whenAll.IsCompleted);
            Debug.Assert(mVal == num + 1);
        }

        private class SomeAwaitable
        {
            public CoroutineAwaiter<Unit> GetAwaiter()
            {
                var t = new TaskCompletionSource<Unit>(sCoroScheduler);
                
                mTs.Add(t);
                return new CoroutineAwaiter<Unit>(sCoroScheduler, t);
            }

            public void SignalWaiters()
            {
                var ts = mTs.ToArray();
                mTs.Clear();
                foreach (var t in ts)
                    t.SetResult(default(Unit));
            }

            private static List<TaskCompletionSource<Unit>> mTs = new List<TaskCompletionSource<Unit>>();
        }

        private static async Task IncrementValueNumTimesAsync(int n, SomeAwaitable someAwaitable)
        {
            for (int i = 0; i < n; ++i)
            {
                sConstraint.Verify();
                Debug.Assert(mVal == i);
                ++mVal;
                await someAwaitable;
                sConstraint.Verify();
            }
        }

        private static void TestAsyncAwait()
        {
            mVal = 0;
            Debug.Assert(mVal == 0);

            var someAwaitable = new SomeAwaitable();
            var num = 40;
            var task = IncrementValueNumTimesAsync(num, someAwaitable);

            for (int i = 0; i < num; ++i)
            {
                Debug.Assert(!task.IsCompleted);
                Debug.Assert(mVal == (i + 1));

                someAwaitable.SignalWaiters();
                sCoroScheduler.Tick();                
            }

            Debug.Assert(task.IsCompleted);
            Debug.Assert(mVal == num);
        }

        private class LetterByLetterAwaitable
        {
            public CoroutineAwaiter<char> GetAwaiter()
            {
                var t = new TaskCompletionSource<char>(sCoroScheduler);

                mTs.Add(t);
                return new CoroutineAwaiter<char>(sCoroScheduler, t);
            }

            public void SignalWaiters(char letter)
            {
                var ts = mTs.ToArray();
                mTs.Clear();
                foreach (var t in ts)
                    t.SetResult(letter);
            }

            private static List<TaskCompletionSource<char>> mTs = new List<TaskCompletionSource<char>>();
        }

        private static async Task<string> LettersToStringAsync(LetterByLetterAwaitable lbl)
        {
            var ls = new List<char>();
            do
                ls.Add(await lbl);
            while (ls.Last() != '\n');

            return new string(ls.ToArray());
        }

        private static void TestAsyncAwaitWithReturnValue()
        {
            var lbl = new LetterByLetterAwaitable();
            var task = LettersToStringAsync(lbl);

            var str = "Ten18\n";
            foreach (var letter in str)
            {
                lbl.SignalWaiters(letter);       
                Debug.Assert(!task.IsCompleted);
                sCoroScheduler.Tick();
            }

            Debug.Assert(task.IsCompleted);
            Debug.Assert(task.Result == str);
        }

        private static async Task<string> WaitOnAllButOneAlreadyCompletedAsync()
        {   
            var ls = new List<char>();

            ls.Add(await CompletedTask<char>.WithValue('U'));
            ls.Add(await CompletedTask<char>.WithValue('n'));
            ls.Add(await CompletedTask<char>.WithValue('i'));
            ls.Add(await sCoroFactory.StartNew(() => { return 'c'; }));
            ls.Add(await CompletedTask<char>.WithValue('o'));
            ls.Add(await CompletedTask<char>.WithValue('r'));
            ls.Add(await CompletedTask<char>.WithValue('n'));
            
            return new string(ls.ToArray());
        }


        private static void TestAsyncAwaitOnAlreadyCompleted()
        {
            var task = WaitOnAllButOneAlreadyCompletedAsync();

            var str = "Unicorn";
            Debug.Assert(!task.IsCompleted);
            sCoroScheduler.Tick();
            Debug.Assert(task.IsCompleted);
            Debug.Assert(task.Result == str);
        }

        static CoroutineTest()
        {
            sCoroFactory = new TaskFactory(sCoroScheduler);            
        }

        private static int mVal = 0;
        private static TaskFactory sCoroFactory; 
        private static CoroutineScheduler sCoroScheduler = new CoroutineScheduler();
        private static SingleThreadedConstraint sConstraint = SingleThreadedConstraint.Create();
    }
}
