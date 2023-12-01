@echo off
title llupdate test
cd /d %~dp0

rem settings
set exe="..\.msvc\x64-Debug\llupdate.exe"
::set exe="llupdate.exe"
set out=~updated.xml
echo %exe%


:select_test
echo.
echo -----------
echo Choose test:
choice /N /C 123q /M "[1]Update ppjs, [2]Update plcprj, [3]No job, [q]Quit: "
goto menu%errorlevel%

:menu0
goto select_test


:menu1
rem Update ppjs
set ppjs=%UserProfile%\Macotec\Machines\m32-Strato\sde\PLC\LogicLab\LogicLab.ppjs
%exe% -v "%ppjs%" -o %out%
echo ---- ret=%errorlevel% ----
"%ProgramFiles%\WinMerge\WinMergeU.exe" /e /u /wl /x "%ppjs%" "%out%"
del "%out%"
goto select_test


:menu2
rem Update plcprj
set plcprj=%UserProfile%\Macotec\Machines\m32-Strato\sde\PLC\LogicLab\LogicLab-plclib.plcprj
%exe% -v "%plcprj%" -o %out%
echo ---- ret=%errorlevel% ----
"%ProgramFiles%\WinMerge\WinMergeU.exe" /e /u /wl /x "%plcprj%" "%out%"
del "%out%"
goto select_test


:menu3
rem Test no job
%exe% -v
echo ---- ret=%errorlevel% ----
goto select_test


:menu4
exit
