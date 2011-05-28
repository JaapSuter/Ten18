#pragma once

namespace Ten18 { namespace Interop {

    class Boolean
    {        
    private:
        std::int32_t mVal;
    
    public:        
        Boolean(bool b) : mVal(static_cast<std::int32_t>(b)) {}
        operator bool () const { return mVal != 0;  }
    };

    static_assert(sizeof(bool) == sizeof(char), "Native C++ bool type must be one byte, as in the CLR, so that booleans can interop.");
    static_assert(sizeof(Boolean) == sizeof(std::int32_t), "Marshal CLR bool type is 4 bytes.");
}}
