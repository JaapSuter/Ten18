#pragma once

namespace Ten18
{
    template<class T>
    class Scoped
    {
    public:
        typedef std::function<void (const T&)> Fun;

        Scoped(const T&& t, const Fun& fun) : mT(t), mFun(fun) {}
        Scoped(Scoped&& rhs) : mT(rhs.mT), mFun(rhs.mFun) { rhs.mFun = Fun(); rhs.mT = T(); }
        ~Scoped() { if (mFun) mFun(mT); }

        operator const T& () const { return mT; }
        
    private:

        Scoped(const Scoped&);
        Scoped& operator = (const Scoped&);

        T mT;
        Fun mFun;
    };

    template<class T>
    Scoped<T> MakeScoped(const T&& t, const typename Scoped<T>::Fun& fun) { return Scoped<T>(std::move(t), fun); }
}
