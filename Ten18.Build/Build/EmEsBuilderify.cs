using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.Security.Policy;
using System.Reflection.Emit;
using System.Reflection;
using System.CodeDom;
using System.CodeDom.Compiler;
using System.Runtime.CompilerServices;
using System.Linq.Expressions;
using System.IO;
using Ten18.Interop;
using Microsoft.Build.Tasks;
using Microsoft.Build.Utilities;
using System.Text;

namespace Ten18.Build
{
    public class EmEsBuilderify : Task
    {
        public override bool Execute()
        {   
            using (var ms = new MemoryStream())
            using (var tw = new StreamWriter(ms))
            {
                Console.SetOut(tw);                
                Console.WriteLine("Blah blah blah");
                Log.LogError("Foobar hallekiedeee");
                
                tw.Flush();
                tw.Close();

                ms.Position = 0;
                using (var tr = new StreamReader(ms))
                    Log.LogMessagesFromStream(tr, Microsoft.Build.Framework.MessageImportance.High);
            }

            return true;
        }
    }
}
