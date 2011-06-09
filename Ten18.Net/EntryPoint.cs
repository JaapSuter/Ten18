using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using Ten18.Async;
using Ten18.Interop;
using Ten18.Capture;

namespace Ten18
{
    static class EntryPoint
    {
        public static async Task Begin(Heart heart)
        {
            using (var cap = new CaptureSource())
            using (var wnd = new Window("Ten18"))
            {                
                wnd.MakeFullScreen();
                
                while (!wnd.HasClosed)
                {
                    await heart.Yield();
                }
            }
        }
    }
}
