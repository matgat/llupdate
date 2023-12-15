@echo off
title run tests
cd /d %~dp0

set test_prj=..\.msvc\llupdate-test.vcxproj
set test_exe=..\.msvc\x64-Debug-test\llupdate-test.exe

:BUILD
rem %PROGRAMFILES%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\msbuild.exe
msbuild %test_prj% -t:Rebuild -p:Configuration=Debug -p:Platform=x64
if %errorlevel% neq 0 goto ERROR

:RUN
if exist %test_exe% (
%test_exe%
if %errorlevel% neq 0 goto ERROR
echo.
timeout /T 3
exit /B 0
)
else (
echo %test_exe% not generated!
goto ERROR
)

:ERROR
echo.
pause
exit /B 1
