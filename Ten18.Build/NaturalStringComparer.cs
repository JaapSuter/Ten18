using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Ten18
{
    class NaturalStringComparer : IComparer<string>
    {
        public int Compare(string a, string b)
        {
            return StrCmpLogicalW(a ?? "", b ?? "");
        }

        [DllImport("shlwapi.dll", CharSet = CharSet.Unicode)]
        private static extern int StrCmpLogicalW(string psz1, string psz2);
    }
}
