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
    static class Util
    {
        public static void Swap<T>(ref T lhs, ref T rhs)
        {
            var tmp = lhs;
            lhs = rhs;
            rhs = tmp;
        }

        public static bool HasDuplicates<T>(this IEnumerable<T> es)
        {
            return null != es.GroupBy(g => g).Where(w => w.Count() > 1).FirstOrDefault();
        }
    }
}
