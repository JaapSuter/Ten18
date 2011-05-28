using System;
using System.Reflection;
using System.Runtime.Hosting;
using System.Security;
using System.Security.Policy;
using System.Threading;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Linq.Expressions;
using System.Diagnostics;
using System.Disposables;
using System.IO;
using System.Text;

namespace Ten18
{
    class DebugTextWriter : TextWriter
    {
        public DebugTextWriter()
        {
            mPrevOut = Console.Out;
            mPrevError = Console.Error;
            Console.SetOut(this);
            Console.SetError(this);
            Debug.AutoFlush = true;
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (this == Console.Out)
                    Console.SetOut(mPrevOut);
                if (this == Console.Error)
                    Console.SetOut(mPrevError);
            }
                
            base.Dispose(disposing);
        }

        public override void Write(char[] buffer, int index, int count) { Debug.Write(new String(buffer, index, count)); }
        public override void Write(string value) { Debug.Write(value); }
        public override Encoding Encoding { get { return Encoding.Default; } }

        private TextWriter mPrevOut;
        private TextWriter mPrevError;
    }
}
