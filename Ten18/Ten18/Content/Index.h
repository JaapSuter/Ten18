#pragma once

namespace Ten18 { namespace Content
{
    class Index
    {
    public:
        
        struct Entry
        {
            const char* Name;
            std::size_t Size;
            const char* Data;
            bool NeedsInteropPatch;
        };
        
        static const Entry* TryGet(const wchar_t* name);
        static const Entry& Get(const wchar_t* name);

    private:

        Index(const Index&);
        Index& operator = (const Index&);
    };
}}
