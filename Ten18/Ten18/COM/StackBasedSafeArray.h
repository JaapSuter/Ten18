#pragma once
#include "Ten18/PCH.h"
namespace Ten18 { namespace COM {

inline SAFEARRAY CreateSafeArrayOnTheStack(const void* ptr, std::size_t size)
{
    SAFEARRAY sa = {};
    static_assert(std::is_pod<SAFEARRAY>::value, "Type must remain a POD, because we clear it using a memset.");
    sa.cDims = 1;
    sa.cbElements = 1;
    sa.pvData = const_cast<void*>(ptr);
    sa.rgsabound[0].cElements = static_cast<ULONG>(size);
    sa.rgsabound[0].lLbound = 0;
    sa.fFeatures = SF_I1 | FADF_AUTO | FADF_EMBEDDED | FADF_FIXEDSIZE;
    return sa;
}

}}
