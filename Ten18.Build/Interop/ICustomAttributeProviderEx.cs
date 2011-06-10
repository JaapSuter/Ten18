using System;
using System.Linq;
using System.Diagnostics;
using Mono.Cecil;
using Mono.Collections.Generic;

namespace Ten18.Interop
{
    static class ICustomAttributeProviderEx
    {
        public static bool HasNativeAttribute(this ICustomAttributeProvider self)
        {
            return self.CustomAttributes.Any(ca => ca.AttributeType.IsSame(Globals.NativeAttribute));
        }

        public static void RemoveNativeAttribute(this ICustomAttributeProvider self)
        {
            var attribs = self.CustomAttributes;
            for (; ; )
            {
                var attrib = attribs.FirstOrDefault(ca => ca.AttributeType.IsSame(Globals.NativeAttribute));
                if (attrib == null)
                    break;
                else
                    attribs.Remove(attrib);
            }

            Debug.Assert(!self.HasNativeAttribute());
        }
    }
}
