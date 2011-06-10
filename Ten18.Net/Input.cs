using System;
using SlimMath;

namespace Ten18
{
    public class Input : IObservable<InputEvent>
    {
        public Input() { }

        public extern Vector2 MousePosition { [Native] get; }

        public IDisposable Subscribe(IObserver<InputEvent> observer)
        {
            throw new NotImplementedException();
        }
    }
}
