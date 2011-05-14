#pragma once
#pragma warning(disable:4127) // conditional expression is constant

#if defined DEBUG
    #error Must not define DEBUG before we do it below
#elif defined NDEBUG
    #if defined _DEBUG
        #error _DEBUG and NDEBUG are mutually exclusive
    #endif
#elif defined _DEBUG
    #define DEBUG
#else
    #error Must define either _DEBUG or NDEBUG
#endif

#if defined _MT
#else
    #error Must define _MT
#endif

#if defined _DLL
#else
    #error Must define _DLL
#endif

#if defined _WIN32
    #define WIN32
#else
    #error Must define _WIN32
#endif

#if defined NTDDI_VERSION
    #error Must not define NTDDI_VERSION before we do it below
#endif

#if defined _WIN32_WINNT
    #error Must not define _WIN32_WINNT before we do it below
#endif

#if defined _SECURE_SCL
    #error Must not define _SECURE_SCL before we do it below
#endif

#if defined _ITERATOR_DEBUG_LEVEL
    #error Must not define _ITERATOR_DEBUG_LEVEL anywhere other than here
#else
    #if defined _DEBUG
        #define _SECURE_SCL 1
        #define _HAS_ITERATOR_DEBUGGING 1
        #define _ITERATOR_DEBUG_LEVEL 2
    #else
        #define _SECURE_SCL 0
        #define _HAS_ITERATOR_DEBUGGING 0
        #define _ITERATOR_DEBUG_LEVEL 0        
    #endif
#endif

#define WINDOWS
#define _WINDOWS

#define UNICODE
#define _UNICODE

#define STRICT
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifdef _DEBUG
    #define Ten18_NEW new(_CLIENT_BLOCK, __FILE__, __LINE__)
    #define Ten18_IF_DEBUG_ELSE(a, b) (a)

    #define _CRTDBG_MAP_ALLOC

    #define D3D_DEBUG_INFO
    #define D3DX10_DEBUG
    
#elif defined NDEBUG
    #define Ten18_NEW new
    #define Ten18_IF_DEBUG_ELSE(a, b) (b)
#else
    #error "Neither _DEBUG or NDEBUG is defined."
#endif


#define NTDDI_VERSION NTDDI_WIN7
#include <sdkddkver.h>

#include <crtdbg.h>
#include <windows.h>
#include <winerror.h>
#include <setupapi.h>
#include <tlhelp32.h>
#include <comdef.h>
#include <shlobj.h>
#include <dxgi.h>
#include <d3d9.h>
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2dbasetypes.h>
#include <d2derr.h>
#include <dwrite.h>
#include <d3dcompiler.h>
#include <dxerr.h>
#include <dwmapi.h>
#include <xnamath.h>
#include <MetaHost.h>
#include <corerror.h>
#include <mscoree.h>
#include <comutil.h>
#include <concrt.h>
#include <ppl.h>
#include <agents.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cwchar>
#include <sstream>
#include <iostream>
#include <memory>
#include <functional>
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>
#include <list>
#include <forward_list>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <array>
#include <vector>
#include <type_traits>
#include <uuids.h>
#include <shlwapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <Dbt.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <ks.h>
#include <ksmedia.h>
#include <fcntl.h>
#include <io.h>

extern "C"
{
    #include <mmsystem.h>
    #include <mmreg.h>
}

#ifdef ReportEvent
    #undef ReportEvent
#endif

#import <mscorlib.tlb> raw_interfaces_only raw_native_types no_implementation

#undef near
#undef far
#undef NOMINMAX
#undef D3D_DEBUG_INFO
#undef D3DX10_DEBUG
#undef WIN32_LEAN_AND_MEAN
#undef STRICT

namespace Ten18
{    
    template <class T>
    inline std::unique_ptr<T> MakeUniquePtr() { return std::unique_ptr<T>(Ten18_NEW T()); }

    template <class T, class A0>
    inline std::unique_ptr<T> MakeUniquePtr(A0&& a0) { return std::unique_ptr<T>(Ten18_NEW T(a0)); }

    template <class T, class A0, class A1>
    inline std::unique_ptr<T> MakeUniquePtr(A0&& a0, A1&& a1) { return std::unique_ptr<T>(Ten18_NEW T(a0, a1)); }

    template <class T, class A0, class A1, class A2>
    inline std::unique_ptr<T> MakeUniquePtr(A0&& a0, A1&& a1, A2&& a2) { return std::unique_ptr<T>(Ten18_NEW T(a0, a1, a2)); }

    template <class T, class A0, class A1, class A2, class A3>
    inline std::unique_ptr<T> MakeUniquePtr(A0&& a0, A1&& a1, A2&& a2, A3&& a3) { return std::unique_ptr<T>(Ten18_NEW T(a0, a1, a2, a3)); }

    template <class T, class A0, class A1, class A2, class A3, class A4>
    inline std::unique_ptr<T> MakeUniquePtr(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) { return std::unique_ptr<T>(Ten18_NEW T(a0, a1, a2, a3, a4)); }
}
