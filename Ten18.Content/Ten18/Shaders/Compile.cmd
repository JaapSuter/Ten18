@echo off

rem %1 = Destination Directory
rem %2 = Debug | Release

cls

del %1*.pso
del %1*.vso

set DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\
set FXC_EXE="%DXSDK_DIR%Utilities\bin\x86\fxc.exe"
set FLAGS=/nologo /WX /Ges /Od /Op /O0 /Zi /Gdp

IF %2% EQU "Release" set FLAGS=/nologo /WX /Ges /O3 /Qstrip_reflect /Qstrip_debug

echo Echo 1: %1
echo Echo 2: %2
echo Echo FLAGS: %FLAGS
echo Echo ~dp0: %~dp0

%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tvs_4_0 /EVS /Fo%1GrayCode.Debug.vso   %~dp0\GrayCode.fx
%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tps_4_0 /EPS /Fo%1GrayCode.Debug.pso   %~dp0\GrayCode.fx

%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tvs_4_0 /EVS /Fo%1Capture.Debug.vso   %~dp0\Capture.fx
%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tps_4_0 /EPS /Fo%1Capture.Debug.pso   %~dp0\Capture.fx
