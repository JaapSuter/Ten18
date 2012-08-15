#pragma once

namespace Ten18
{
    class OnExit
    {
    public:
        typedef std::function<void ()> Fun;

        OnExit(const Fun& fun) : mFun(fun) {}
        ~OnExit() { mFun(); }
        
    private:

        OnExit(const OnExit&);
        OnExit& operator = (const OnExit&);

        Fun mFun;
    };
}
