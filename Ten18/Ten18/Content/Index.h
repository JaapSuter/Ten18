#pragma once

#include "Ten18/Memory.h"

namespace Ten18 { namespace Content
{
    class Index
    {
    public:

        static const wchar_t* Root();
        
        struct Entry
        {
            const char* Name;
            std::size_t Size;
            const char* Data;
        };

        struct ScopedEntry : Entry
        {
            explicit ScopedEntry(const Entry& self)
                : Entry(self), mIsEmbedded(true) {}

            ScopedEntry(size_t size, char* data)
                : Entry(), mIsEmbedded(false) { Name = "No Name"; Size = size; Data = data; }

            ScopedEntry(ScopedEntry&& other) : Entry(other), mIsEmbedded(other.mIsEmbedded) { other.Data = nullptr; }
            
            ~ScopedEntry()
            {
                if (mIsEmbedded)
                    return;
                
                if (!Data)
                    Memory::Free(const_cast<char*>(Data));
                Data = nullptr;
            }
            
        private:
            ScopedEntry& operator = (const ScopedEntry&);
            bool mIsEmbedded;
        };

        static const Entry* TryGet(const wchar_t* name);
        static const ScopedEntry Get(const wchar_t* name);

    private:

        Index(const Index&);
        Index& operator = (const Index&);
    };
}}
