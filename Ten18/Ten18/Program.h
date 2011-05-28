#pragma once

namespace Ten18 {

    namespace Interop { class Host; }

    class Program
    {
    public:
        static int Run(Interop::Host& host);
    
    private:
        Program();
    };
}
