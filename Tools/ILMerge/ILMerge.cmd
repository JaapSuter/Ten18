@echo off
cls
ilmerge.exe /lib:"C:\Program Files (x86)\Microsoft Cloud Programmability\Reactive Extensions\v1.0.2856.0\Net4" ^
            /lib:"C:\Windows\Microsoft.NET\Framework\v4.0.30319\WPF" ^
            /lib:"..\..\obj\Win32\Debug" ^
            /keyfile:..\..\Ten18.snk ^
            /target:library ^
            /wildcards ^
            /log ^
            /out:..\..\obj\Win32\Debug\Ten18.Net.Merged.dll ^
            /targetplatform:v4,C:\Windows\Microsoft.NET\Framework\v4.0.30319 ^
            Ten18.Net.dll ^
            Ten18.Interop.Core.dll ^
            Ten18.Interop.Core.Interop.dll ^
            AsyncCtpLibrary.dll ^
            SlimMath.dll
            



