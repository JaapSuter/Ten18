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

        /*
<# foreach (var methodGenerator in mMethodGenerators) { #>
     <#= methodGenerator.ExportName #> @<#= methodGenerator.Ordinal #>  NONAME PRIVATE
<# } #>
        */
        internal int RegisterForOrdinal(MethodGenerator methodGenerator)
        {
            mMethodGenerators.Add(methodGenerator);
            return mMethodGenerators.Count;
        }

        public void Generate()            
        {
            var file = Path.Combine(Paths.WorkingDir, @"Ten18.def");
            var contents = TransformText();
            File.WriteAllText(file, contents);

            Console.WriteLine("Updated: {0}", file);
        }

        private IList<MethodGenerator> mMethodGenerators = new List<MethodGenerator>();

        // private ISet<string> mIncludeFiles = new HashSet<string>();
        // mIncludeFiles.Add(methodGenerator.DeclaringType.FullNameInCpp().Trim(':').Replace("::", "/") + ".h");        
    }
}
