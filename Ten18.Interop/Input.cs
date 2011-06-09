using System;
using System.Globalization;
using System.Runtime.InteropServices;
using Ten18.Interop;
using SlimMath;

namespace Ten18
{
    public abstract class Input : IObservable<InputEvent>
    {
        public Input() { }

        public abstract Vector2 MousePosition { get; }

        public IDisposable Subscribe(IObserver<InputEvent> observer)
        {
            throw new NotImplementedException();
        }
    }
}
