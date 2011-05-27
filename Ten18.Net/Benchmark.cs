using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;

namespace Ten18
{
    class Benchmark
    {
        public string Name { get; private set; }
        public long ElapsedTicks { get; private set; }
        public long ElapsedMilliseconds { get; private set; }
        
        public override string ToString()
        {
            var padRight = 68;
            return String.Format("Benchmark(\"{0}\"):".PadRight(padRight) + "{1,8} ticks, {2,4} ms", Name, ElapsedTicks, ElapsedMilliseconds);
        }

        public Benchmark(string name, Action action)
        {
            Name = name;

            var sw = Stopwatch.StartNew();
            action();
            sw.Stop();
            
            ElapsedTicks = sw.ElapsedTicks;
            ElapsedMilliseconds = sw.ElapsedMilliseconds;
        }

        public static void Run(string name, Action action)
        {
            Console.WriteLine(new Benchmark(name, action));
        }
    }
}
