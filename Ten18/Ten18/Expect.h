#pragma once

#include "Ten18/Assert.h"

namespace Ten18 {

    class ExpectImpl
	{
    public:
        
        template<class T>
        struct EqualToImpl
        {
            T mT;
            EqualToImpl(const T& t) : mT(t) {}
            
            template <class T>        
            const T& operator = (const T& t)
            {
                Ten18_VERIFY_LAST_ERROR(mT == t);
                return t;
            }

        private:
            EqualToImpl& operator = (const EqualToImpl&);
        };
        
        template<class T>
        struct NotEqualToImpl
        {
            T mT;
            NotEqualToImpl(const T& t) : mT(t) {}
            
            template <class T>        
            const T& operator = (const T& t)
            {
                Ten18_VERIFY_LAST_ERROR(static_cast<T>(mT) != t);
                return t;
            }

        private:
            NotEqualToImpl& operator = (const NotEqualToImpl&);
        };

        struct TrueImpl
        {
            template <class T>
            const T& operator = (const T& t)
            {
                Ten18_VERIFY_LAST_ERROR(!!(t));
                return t;
            }

        private:
            TrueImpl& operator = (const TrueImpl&);
        };

        struct HResultImpl
        {
            HRESULT operator = (const HRESULT& hr)
            {
                Ten18_VERIFY_HR(hr);
                return hr;
            }
        };

    public:

        ExpectImpl() : Zero(0), NotZero(0), NotNull(nullptr) {}

        HResultImpl                       HR;
        TrueImpl                          True;
        EqualToImpl<int>                  Zero;
        NotEqualToImpl<int>               NotZero;
        NotEqualToImpl<decltype(nullptr)> NotNull;

        template<class T>
        NotEqualToImpl<T>                 NotEqualTo(const T& t) { return NotEqualToImpl<T>(t); }
        
        template<class T>
        EqualToImpl<T>                    EqualTo(const T& t) { return EqualToImpl<T>(t); }       
        
    private:

        ExpectImpl(const ExpectImpl&);
        ExpectImpl& operator = (const ExpectImpl&);
    };
    
    struct IgnoreImpl
    {
        template<class T>
        const T& operator = (const T& t) { return t; }
    };

	extern IgnoreImpl Ignore;
    extern ExpectImpl Expect;   // Todo, make throwing;
    extern ExpectImpl HopeFor;  // Todo, make non-throwing;
}
