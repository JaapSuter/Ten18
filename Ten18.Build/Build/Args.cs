using System;
using System.Diagnostics;
using System.Linq;

namespace Ten18
{
    static class Args
    {
        public static void Set(string[] args)
        {
            sArgs = args;
        }

        public static T Get<T>(string name, T fallback = default(T))
        {
            return Get<T>(name, () => fallback);
        }

        public static T Get<T>(string name, Func<T, bool> predicate, T fallback)
        {
            return Get<T>(name, val => true, () => fallback);
        }

        public static T Get<T>(string name, Func<T> fallback)
        {
            return Get<T>(name, val => true, fallback);
        }

        public static T Get<T>(string name, Func<T, bool> predicate, Func<T> fallback)
        {
            // Less than optimal? You bet...!
            var prefix = '-';
            var upToArg = sArgs.SkipWhile(arg => arg[0] != prefix || !arg.EndsWith(name));
            
            if (typeof(T) == typeof(bool))
                return UglyCast<T>(!upToArg.IsEmpty());
            else if (typeof(T) == typeof(string))
                return UglyCast<T>(upToArg.Skip(1).FirstOrDefault(val => predicate((T)(object)val)) ?? UglyCast<string>(fallback()));
            
            Debug.Fail(String.Format("Commandline Argument '{0}' Has Unsupported Type '{1}'", name, typeof(T).FullName));
            return default(T);

        }

        private static To UglyCast<To>(object from) { return (To)from; }

        private static string[] sArgs;
    }
}
