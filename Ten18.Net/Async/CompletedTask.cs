using System.Threading.Tasks;

namespace Ten18.Async
{
    public static class CompletedTask
    {
        public readonly static Task Default = CompletedTask<object>.Default;
    }

    public class CompletedTask<TResult>
    {
        public static Task<TResult> WithValue(TResult tr)
        {
            var tcs = new TaskCompletionSource<TResult>();
            tcs.TrySetResult(tr);
            return tcs.Task;
        }

        static CompletedTask()
        {
            var tcs = new TaskCompletionSource<TResult>();
            tcs.TrySetResult(default(TResult));
            Default = tcs.Task;
        }

        public readonly static Task<TResult> Default;
    }
}
