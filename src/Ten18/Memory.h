#pragma once

namespace Ten18
{
    class Memory
    {
    public:
        enum Tag {};

            static void Initialize();
            static void Shutdown();

        static void* Alloc(std::size_t size, const char* file, int line);
        static void Free(void*);

    private:
        Memory();
        Memory(const Memory&);
        Memory& operator = (const Memory&);
    };

    #define Ten18_NO_DEFAULT_OPERATOR_NEW_DELETE           \
        private:                                           \
            static void * operator new (size_t);           \
            static void * operator new [] (size_t size);
        
    #define Ten18_CUSTOM_OPERATOR_NEW_DELETE                                                                                                                          \
        Ten18_NO_DEFAULT_OPERATOR_NEW_DELETE                                                                                                                          \
        public:                                                                                                                                                       \
            static inline void * operator new (size_t size, ::Ten18::Memory::Tag, const char* file, int line) { return ::Ten18::Memory::Alloc(size, file, line); }    \
            static inline void * operator new [] (size_t size, ::Ten18::Memory::Tag, const char* file, int line) { return ::Ten18::Memory::Alloc(size, file, line); } \
            static inline void operator delete (void* ptr, ::Ten18::Memory::Tag, const char*, int) { return ::Ten18::Memory::Free(ptr); }                             \
            static inline void operator delete [] (void* ptr, ::Ten18::Memory::Tag, const char*, int) { return ::Ten18::Memory::Free(ptr); }                          \
            static inline void operator delete (void* ptr) { return ::Ten18::Memory::Free(ptr); }                                                                     \
            static inline void operator delete [] (void* ptr) { return ::Ten18::Memory::Free(ptr); }                                                                  \
        private:

    #define Ten18_NEW \
        new (::Ten18::Memory::Tag(), __FILE__, __LINE__)

}
