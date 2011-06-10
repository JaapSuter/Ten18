using System;

namespace Ten18
{
    [AttributeUsage(AttributeTargets.Method | AttributeTargets.Struct | AttributeTargets.Enum | AttributeTargets.Property)]
    public class NativeAttribute : Attribute
    {
    }
}
