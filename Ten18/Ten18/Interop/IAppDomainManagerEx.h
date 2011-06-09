#pragma once
#include "Ten18/COM/ImplUnknown.h"
#include "Ten18/COM/COMPtr.h"
#include "Ten18/Interop/HostMalloc.h"

namespace Ten18 {
    
    namespace Interop {
                    
    [uuid("37D42543-4326-4A49-9320-A1BF1716AB54")]
    __interface __declspec(novtable) IAppDomainManagerEx : IUnknown
    {
        virtual void __stdcall Rendezvous() = 0;
        virtual void __stdcall Tick() = 0;
        virtual void __stdcall Farewell() = 0;
    };

}}
