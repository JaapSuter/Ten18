#pragma once

#include "Ten18/CLR/HostControl.h"
#include "Ten18/CLR/HostAssemblyManager.h"
#include "Ten18/CLR/HostAssemblyStore.h"
#include "Ten18/CLR/HostMemoryManager.h"

namespace Ten18 { namespace CLR {

    static_assert(sizeof(bool) == sizeof(char), "Native C++ bool type must be one byte, as in the CLR, so that booleans can interop.");

    class Host
    {
    public:
        
        Host();

        void RendezVous();

    private:
        
        Host(const Host&);
        Host& operator = (const Host&);

        friend class HostControl;
        friend class HostAssemblyManager;
        friend class HostAssemblyStore;
        friend class HostMemoryManager;  

        HostControl         mControl;
        HostAssemblyManager mAssemblyManager;
        HostAssemblyStore   mAssemblyStore;
        HostMemoryManager   mMemoryManager;

        IAppDomainManagerEx* mAppDomainManagerEx;
        
        // Because a process that hosts the CLR can't actually stop, unload, or otherwise deinitialize
        // the CLR (or its default AppDomain) once loaded, we just let it die with the process (which appears
        // to by as designed by the CLR hosting API). Henceforth, we're not going to bother releasing any of
        // these pointers. And that's why we don't use reference counted smart pointers, and just resort to
        // plain old fashioned raw pointers.
        ICLRMetaHost*       mMetaHost;
        ICLRRuntimeInfo*    mRuntimeInfo;
        ICLRControl*        mClrControl;
        ICLRGCManager*      mGCManager;     
        ICLRRuntimeHost*    mRuntimeHost;
    };
}}
