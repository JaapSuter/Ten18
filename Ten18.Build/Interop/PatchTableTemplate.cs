using System.Collections.Generic;
using System.Linq;
using System;
using System.IO;
using Ten18.Build;

namespace Ten18.Interop
{
    partial class PatchTableTemplate
    {
        public PatchTableTemplate()
        {
        }

        public void Generate()            
        {
            var cppHeaderFile = Path.Combine(Paths.WorkingDir, @"Ten18\Interop\PatchTable.Generated.h");
            if (!Directory.Exists(Path.GetDirectoryName(cppHeaderFile)))
                Directory.CreateDirectory(Path.GetDirectoryName(cppHeaderFile));
        
            mNativeSignatures = mNativeSignatures.OrderBy(ns => ns.Cookie).ToList();
            
            var code = TransformText();
            File.WriteAllText(cppHeaderFile, code);

            Console.WriteLine("Updated: {0}", cppHeaderFile);
        }

        internal void Add(NativeSignature nativeSignature)
        {
            mNativeSignatures.Add(nativeSignature);
            mIncludeFiles.Add(nativeSignature.DeclaringType.FullNameInCpp().Trim(':').Replace("::", "/") + ".h");
        }

        private IList<NativeSignature> mNativeSignatures = new List<NativeSignature>();
        private ISet<string> mIncludeFiles = new HashSet<string>();
    }
}
