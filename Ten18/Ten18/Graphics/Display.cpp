#include "Ten18/Graphics/Display.h"
#include "Ten18/Expect.h"
#include "Ten18/Util.h"

using namespace Ten18;
using namespace Ten18::Graphics;

void Display::EnumerateUsingQueryDisplayConfig()
{
    UINT32 numPaths = 0;
    UINT32 numModes = 0;

    Expect.EqualTo(ERROR_SUCCESS) = GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &numPaths, &numModes);

    const auto reasonableOverEstimateForMaximumNumberOfExtraDisplaysOneMightAddInThisNanosecondRaceConditionGeez = 2;
    numPaths *= reasonableOverEstimateForMaximumNumberOfExtraDisplaysOneMightAddInThisNanosecondRaceConditionGeez;
    numModes *= reasonableOverEstimateForMaximumNumberOfExtraDisplaysOneMightAddInThisNanosecondRaceConditionGeez;

    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    modes.resize(numModes);
    paths.resize(numPaths);
    Expect.EqualTo(ERROR_SUCCESS) = QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPaths, &paths[0], &numModes, &modes[0], nullptr);

    std::for_each(paths.begin(), paths.end(), [&] (const DISPLAYCONFIG_PATH_INFO& pi)
    {   
        DISPLAYCONFIG_SOURCE_DEVICE_NAME sdn = {};
        sdn.header.size = sizeof(sdn);
        sdn.header.adapterId = pi.sourceInfo.adapterId;
        sdn.header.id = pi.sourceInfo.id;
        sdn.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        Expect.Zero = DisplayConfigGetDeviceInfo(&sdn.header);
        DebugOut("Source: %S", sdn.viewGdiDeviceName);

        DISPLAYCONFIG_TARGET_DEVICE_NAME tdn = {};        
        tdn.header.size = sizeof(tdn);
        tdn.header.adapterId = pi.targetInfo.adapterId;
        tdn.header.id = pi.targetInfo.id;
        tdn.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&tdn.header))
        {
            DebugOut("Target: %S", tdn.monitorFriendlyDeviceName);
            DebugOut("        %S", tdn.monitorDevicePath);
        }
        else
        {
            DebugOut("Target: %S", L"Error");
        }
        
        #define Ten18_TRACE_OutputTechnology(x) if (pi.targetInfo.outputTechnology == x) DebugOut("    %s\n", #x)
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER                            );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15                             );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO                           );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO                  );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO                  );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI                              );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI                             );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS                             );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN                            );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI                              );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL             );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED             );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL                     );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED                     );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDTVDONGLE                       );
        Ten18_TRACE_OutputTechnology(DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL                         );
        #define Ten18_TRACE_StatusFlags(x) DebugOut("    %s: %d\n", #x, (pi.targetInfo.statusFlags & x))
        Ten18_TRACE_StatusFlags(DISPLAYCONFIG_TARGET_IN_USE);
        Ten18_TRACE_StatusFlags(DISPLAYCONFIG_TARGET_FORCIBLE);
        // Ten18_TRACE_StatusFlags(DISPLAYCONFIG_TARGET_FORCED_AVAILABILITY_BOOT);
        // Ten18_TRACE_StatusFlags(DISPLAYCONFIG_TARGET_FORCED_AVAILABILITY_PATH);
        // Ten18_TRACE_StatusFlags(DISPLAYCONFIG_TARGET_FORCED_AVAILABILITY_SYSTEM);
    });
}

static void EnumerateUsingEnumDisplayDevices()
{
    DISPLAY_DEVICE dd = {};
    dd.cb = sizeof(dd);

    for (DWORD dev = 0; EnumDisplayDevices(0, dev, &dd, 0); ++dev)
    {
        std::wprintf(L"[%d] GraphicsDevice Flags: 0x%X\n", dev, dd.StateFlags);                    
        std::wprintf(L"\tKey: %s\n", dd.DeviceKey);
        std::wprintf(L"\tName: %s\n", dd.DeviceName);
        std::wprintf(L"\tString: %s\n", dd.DeviceString);

        DISPLAY_DEVICE ddMon = {};
        ddMon.cb = sizeof(ddMon);
            
        for (DWORD mon = 0; EnumDisplayDevices(dd.DeviceName, mon, &ddMon, 0); ++mon)
        {
            if (ddMon.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)
                    continue;

            if (ddMon.StateFlags & DISPLAY_DEVICE_ACTIVE)
                std::wprintf(L"\t[%d/%d] Active: %s\n", dev, mon, ddMon.DeviceID);
            else
                std::wprintf(L"\t[%d/%d] Not Active: %s\n", dev, mon, ddMon.DeviceID);
                    
            std::wprintf(L"\t\tKey: %s\n", ddMon.DeviceKey);
            std::wprintf(L"\t\tName: %s\n", ddMon.DeviceName);
            std::wprintf(L"\t\tString: %s\n", ddMon.DeviceString);

            ZeroMemory(&ddMon, sizeof(ddMon));
            ddMon.cb = sizeof(ddMon);
        }

        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
    }
}

Display::Display()
{
}

Display::Display(Display&& other)    
{
    Out = std::move(other.Out);
    Desc = other.Desc;
    InfoEx = other.InfoEx;
    other.InfoEx = MONITORINFOEX();
    other.Desc = DXGI_OUTPUT_DESC();
}

Display& Display::operator = (Display&& other)
{
    Out = std::move(other.Out);
    Desc = other.Desc;
    InfoEx = other.InfoEx;
    other.InfoEx = MONITORINFOEX();
    other.Desc = DXGI_OUTPUT_DESC();
    return *this;
}

void Display::Enumerate(IDXGIFactory1& dxgiFactory1, Display::List& dst)
{
    auto no = false;
    if (no)
        EnumerateUsingEnumDisplayDevices();
    
    dst.clear();
    for (UINT i = 0; ; ++i)
    {
        COM::COMPtr<IDXGIAdapter1> dxgiAdapter;
        auto hr = dxgiFactory1.EnumAdapters1(i, dxgiAdapter.AsTypedDoubleStar());
        if (DXGI_ERROR_NOT_FOUND == hr)
            break;
        else
            Expect.HR = hr;

        DXGI_ADAPTER_DESC1 desc1 = {};
        Expect.HR = dxgiAdapter->GetDesc1(&desc1);

        for (UINT j = 0; ; ++j)
        {
            Display output;
            hr = dxgiAdapter->EnumOutputs(j, output.Out.AsTypedDoubleStar());
            if (DXGI_ERROR_NOT_FOUND == hr)
                break;
            else
                Expect.HR = hr;

            Expect.HR = output.Out->GetDesc(&output.Desc);

            output.InfoEx.cbSize = sizeof(output.InfoEx);
            Expect.True = GetMonitorInfo(output.Desc.Monitor, &output.InfoEx);

            if (MONITORINFOF_PRIMARY & output.InfoEx.dwFlags)
                dst.emplace_front(std::move(output));
            else
                dst.emplace_back(std::move(output));
        }
    }
}

