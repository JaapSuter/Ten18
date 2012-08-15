#include "Ten18/PCH.h"
#include "Ten18/RawInput.h"
#include "Ten18/Expect.h"
#include "Ten18/Assert.h"

using namespace Ten18;
using namespace Ten18::Input;

LRESULT RawInput::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT num = 0;
    Ten18_EXPECT.Not(UINT(-1)) = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &num, sizeof(RAWINPUTHEADER));

    const UINT max = 1024;
    BYTE buffer[max];
    Ten18_ASSERT(num < max);

    Ten18_EXPECT.EqualTo(UINT(num)) = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, buffer, &num, sizeof(RAWINPUTHEADER));

    const RAWINPUT* ris = static_cast<const RAWINPUT*>(static_cast<void*>(buffer));

    if (ris->header.dwType == RIM_TYPEKEYBOARD)
    {
        const auto& kb = ris->data.keyboard;
        const auto key = kb.VKey;
        const bool released = kb.Flags & RI_KEY_BREAK;
        if (key == VK_SPACE && released)
        {
            MessageBox(nullptr, L"Space Release", L"Yo dawg...", MB_OK);
        }
    }
    else if (ris->header.dwType == RIM_TYPEMOUSE)
    {
        // int dx = ris->data.mouse.lLastX;
        // int dy = ris->data.mouse.lLastY;
    } 

    const auto wParamAsRim = GET_RAWINPUT_CODE_WPARAM(wParam);
    if (wParamAsRim == RIM_INPUT || wParamAsRim == RIM_INPUTSINK)
        return ::DefWindowProc(hwnd, msg, wParam, lParam);

    return 0;
}

void RawInput::Register(HWND hwnd)
{
    UINT num = 0;
    UINT rum = 0;
    Ten18_EXPECT.Not(UINT(-1)) = GetRawInputDeviceList(nullptr, &num, sizeof(RAWINPUTDEVICELIST));
    
    const UINT max = 27;
    RAWINPUTDEVICELIST dls[max] = {};
    RAWINPUTDEVICE rids[max] = {};
    Ten18_ASSERT(num < max);
    
    Ten18_EXPECT.EqualTo(UINT(num)) = GetRawInputDeviceList(dls, &num, sizeof(RAWINPUTDEVICELIST));
         
    for (UINT i = 0; i < num; ++i)
    {         
        RID_DEVICE_INFO rdi = {};
        UINT size = rdi.cbSize = sizeof(RID_DEVICE_INFO);        
        
        Ten18_EXPECT.EqualTo(sizeof(rdi)) = GetRawInputDeviceInfo(dls[i].hDevice, RIDI_DEVICEINFO, &rdi, &size);

        if (rdi.dwType == RIM_TYPEMOUSE || rdi.dwType == RIM_TYPEKEYBOARD)
        {
            rids[rum].usUsagePage = rdi.hid.usUsagePage;
            rids[rum].usUsage = rdi.hid.usUsage;
            rids[rum].hwndTarget = hwnd;
            rids[rum].dwFlags = 0;

            const USHORT magic1 = 1, magic2 = 0x2, magic3 = 0x6;
            rids[rum].usUsagePage = magic1; 
            rids[rum].usUsage = rdi.dwType == RIM_TYPEMOUSE ? magic2 : magic3;
            
            ++rum;            
        }
    }

    Ten18_EXPECT.EqualTo(TRUE) = RegisterRawInputDevices(rids, rum, sizeof(rids[0]));
}
