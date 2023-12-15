@echo off
title Build
cd /d %~dp0

rem %PROGRAMFILES%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\msbuild.exe
msbuild llupdate.vcxproj -t:Build -p:Configuration=Debug -p:Platform=x64
if %errorlevel% neq 0 (
pause
exit /B 1
)
