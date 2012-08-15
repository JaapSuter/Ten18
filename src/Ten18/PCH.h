#pragma once
#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4355) // 'this' : used in base member initializer list

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
    #define Ten18_DEBUG
    #define Ten18_IF_DEBUG_ELSE(a, b) (a)
    
    // From MSDN: for the CRT functions to work correctly when using _CRTDBG_MAP_ALLOC, the #include statements must follow the order shown here...
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
    
    #define D3D_DEBUG_INFO
    
#elif defined NDEBUG
    #define Ten18_IF_DEBUG_ELSE(a, b) (b)
#else
    #error "Neither _DEBUG or NDEBUG is defined."
#endif

#define NTDDI_VERSION NTDDI_WIN8
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
#include <d3d11_1.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d2dbasetypes.h>
#include <d2derr.h>
#include <dwrite.h>
#include <dwrite_1.h>
#include <d3dcompiler.h>
#include <dwmapi.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <comutil.h>
#include <concrt.h>
#include <ppl.h>
#include <agents.h>
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
#include <wincodec.h>
#include <dbghelp.h>

extern "C"
{
    #include <mmsystem.h>
    #include <mmreg.h>
}

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <deque>
#include <filesystem>
#include <forward_list>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <regex>
#include <sstream>
#include <stack>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "MicrosoftSDL/Banned.h"

#ifdef ReportEvent
    #undef ReportEvent
#endif

#undef near
#undef far
#undef NOMINMAX
#undef D3D_DEBUG_INFO
#undef WIN32_LEAN_AND_MEAN
#undef STRICT

namespace dx = DirectX;
