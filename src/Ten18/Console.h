#pragma once

namespace Ten18
{
    class Console
    {
    public:

        static void Initialize(const wchar_t* title);        
        static void Flush();        
        static void WriteLine(const char* fmt);
        
    private:

        Console();
        Console(const Console&);
        Console& operator = (const Console&);
    };
}
