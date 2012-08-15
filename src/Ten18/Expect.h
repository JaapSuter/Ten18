#pragma once

#include "Ten18/Assert.h"

namespace Ten18 {

    class ExpectImpl
    {
    private:

        template<class T>
        struct EqualToImpl
        {   
            static_assert(std::is_scalar<T>::value, "");
            T mT;
            EqualToImpl(T t) : mT(t) {}
            
            template <class T>        
            T operator = (T t)
            {
                if (static_cast<T>(mT) != t)
                    if (Ten18::Assert::ReportFailure(nullptr, tSourceFile, tLineNumber, nullptr, S_OK, GetLastError()))
                        __debugbreak();
                return t;
            }

        private:
            EqualToImpl& operator = (const EqualToImpl&);
        };
        
        template<class T>
        struct NotEqualToImpl
        {
            static_assert(std::is_scalar<T>::value, "");
            T mT;
            NotEqualToImpl(T t) : mT(t) {}
            
            template <class T>        
            T operator = (T t)
            {
                if (static_cast<T>(mT) == t)
                    if (Ten18::Assert::ReportFailure(nullptr, tSourceFile, tLineNumber, nullptr, S_OK, GetLastError()))
                        __debugbreak();
                return t;
            }

        private:
            NotEqualToImpl& operator = (const NotEqualToImpl&);
        };

        struct TrueImpl
        {
            template <class T>
            T operator = (T t)
            {
                static_assert(std::is_scalar<T>::value, "");
                if (!t)
                    if (Ten18::Assert::ReportFailure(nullptr, tSourceFile, tLineNumber, nullptr, S_OK, GetLastError()))
                        __debugbreak();
                
                return t;
            }

        private:
            TrueImpl& operator = (const TrueImpl&);
        };

        struct HResultImpl
        {
            HRESULT operator = (HRESULT hr)
            {
                if (FAILED(hr))
                    if (Ten18::Assert::ReportFailure(nullptr, tSourceFile, tLineNumber, nullptr, hr))
                        __debugbreak();
		    
                return hr;
            }
        };

    public:

        HResultImpl           HR;
        TrueImpl              True;
        EqualToImpl<int>      Zero;
        NotEqualToImpl<int>   NotZero;
        NotEqualToImpl<void*> NotNull;
                              
        template<class T>     
        NotEqualToImpl<T>     Not(T t) { return NotEqualToImpl<T>(t); }
                              
        template<class T>     
        EqualToImpl<T>        EqualTo(T t) { return EqualToImpl<T>(t); }

    public:
        inline ExpectImpl&    SetThreadSpecificFileAndLine(const char* file, int line) { tSourceFile = file; tLineNumber = line; return *this; }
        
        ExpectImpl() : Zero(0), NotZero(0), NotNull(nullptr) {}    

    private:

        ExpectImpl(const ExpectImpl&);
        ExpectImpl& operator = (const ExpectImpl&);

        __declspec(thread) static const char* tSourceFile;
        __declspec(thread) static int tLineNumber;        
    };
    
    struct IgnoreImpl
    {
        template<class T>
        const T& operator = (const T& t) { return t; }
    };

    extern ExpectImpl Expect;   // Todo, Jaap Suter, August 2012, make throwing;
    extern ExpectImpl HopeFor;  // Todo, Jaap Suter, August 2012, make non-throwing;
    extern IgnoreImpl Ignore;
}

#ifdef Ten18_DEBUG
    #define Ten18_EXPECT   ::Ten18::Expect.SetThreadSpecificFileAndLine(__FILE__, __LINE__)        
    #define Ten18_HOPE_FOR ::Ten18::HopeFor.SetThreadSpecificFileAndLine(__FILE__, __LINE__)
    #define Ten18_IGNORE   ::Ten18::Ignore
#else
    #define Ten18_EXPECT   ::Ten18::Expect
    #define Ten18_HOPE_FOR ::Ten18::HopeFor
    #define Ten18_IGNORE   ::Ten18::Ignore
#endif

