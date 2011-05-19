#include "Ten18/PCH.h"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "mscoree.lib")

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3d10_1.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Wmvcore.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "dxva2.lib")

#ifdef _DEBUG    
    #pragma comment(lib, "d3dx9d.lib")
    #pragma comment(lib, "d3dx10d.lib")
    #pragma comment(lib, "d3dx11d.lib")
#else    
    #pragma comment(lib, "d3dx9.lib")
    #pragma comment(lib, "d3dx10.lib")
    #pragma comment(lib, "d3dx11.lib")
#endif


#pragma comment(lib, "../Libraries/CLEyeMulticam.lib")