#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/CLR/HostMalloc.h"

namespace Ten18 {
    
    namespace CLR {

    struct __declspec(uuid("37D42543-4326-4A49-9320-A1BF1716AB54")) IAppDomainManagerEx : IUnknown
    {
        virtual void __stdcall RendezVous();
    };
}}
