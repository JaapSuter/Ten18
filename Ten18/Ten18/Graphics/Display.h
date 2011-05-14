#pragma once

#include "Ten18/COM/COMPtr.h"

namespace Ten18
{
    namespace Graphics {

        class Display
        {
        public:

            typedef std::list<const Display> List;

            static void Enumerate(IDXGIFactory1& dxgiFactory1, Display::List& dst);

            static void EnumerateUsingQueryDisplayConfig();

            COM::COMPtr<IDXGIOutput> Out;
            DXGI_OUTPUT_DESC Desc;
            MONITORINFOEX InfoEx;           
            
            Display();
            Display(Display&&);
            Display& operator = (Display&&);
            
        private:
            Display(const Display&);
            Display& operator = (const Display&);
        };
}}