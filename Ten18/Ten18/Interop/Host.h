#pragma once

#include "Ten18/Interop/HostControl.h"
#include "Ten18/Interop/HostAssemblyManager.h"
#include "Ten18/Interop/HostAssemblyStore.h"
#include "Ten18/Interop/HostMemoryManager.h"
#include "Ten18/Interop/HostGCManager.h"

namespace Ten18 { namespace Interop {

    class Host
    {
        static_assert(sizeof(bool) == sizeof(char), "");
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "");
        static_assert(sizeof(std::int16_t) == sizeof(short), "");
        static_assert(sizeof(std::int32_t) == sizeof(int), "");
        static_assert(sizeof(std::int64_t) == sizeof(__int64), "");
        static_assert(sizeof(std::uint16_t) == sizeof(unsigned short), "");
        static_assert(sizeof(std::uint32_t) == sizeof(unsigned int), "");
        static_assert(sizeof(std::uint64_t) == sizeof(unsigned __int64), "");

    public:
        
        Host();
        ~Host();

        void Tick();
        
        __declspec(noreturn) void Exit(int exitCode);

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
        HostGCManager       mHostGCManager;

        // Because a process that hosts the CLR can't actually stop, unload, or otherwise deinitialize
        // the CLR (or its default AppDomain) once loaded, we just let it die with the process (which appears
        // to by as designed by the CLR hosting API). Henceforth, we're not going to bother releasing any of
        // these pointers. And that's why we don't use reference counted smart pointers, and just resort to
        // plain old fashioned raw pointers.
        IAppDomainManagerEx*    mAppDomainManagerEx;
        ICLRMetaHost*           mMetaHost;
        ICLRRuntimeInfo*        mRuntimeInfo;
        ICLRControl*            mClrControl;
        ICLRGCManager*          mGCManager;
        ICLRRuntimeHost*        mRuntimeHost;
    };
}}
