@echo off

set CONFIG=Debug

cls

del %1*.pso
del %1*.vso

set DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\
set FXC_EXE="%DXSDK_DIR%Utilities\bin\x86\fxc.exe"
set FLAGS=/nologo /WX /Ges /Od /Op /O0 /Zi /Gdp

IF %CONFIG% EQU "Release" set FLAGS=/nologo /WX /Ges /O3 /Qstrip_reflect /Qstrip_debug

%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tvs_4_0 /EVS /Fo%~dp0\GrayCode.vso   %~dp0\GrayCode.fx
%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tps_4_0 /EPS /Fo%~dp0\GrayCode.pso   %~dp0\GrayCode.fx

%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tvs_4_0 /EVS /Fo%~dp0\Capture.vso   %~dp0\Capture.fx
%FXC_EXE% %FLAGS% %DEBUG_FLAGS%   /Tps_4_0 /EPS /Fo%~dp0\Capture.pso   %~dp0\Capture.fx
