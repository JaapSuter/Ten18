#pragma once

namespace Ten18 { namespace Graphics {

    template<class T>
    static void SetDebugName(const T& t, const char* name)
    {
        #ifdef Ten18_DEBUG
            if (t)
                t->SetPrivateData(WKPDID_D3DDebugObjectName, std::strlen(name), name);
        #else
            Ten18_UNUSED(t);
            Ten18_UNUSED(name);
        #endif
    }
}}
