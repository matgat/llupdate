$test_prj = "..\.msvc\llupdate-test.vcxproj"
$test_exe = "..\.msvc\x64-Debug-test\llupdate-test.exe"
$ErrorActionPreference = "Inquire"
Set-Location $PSScriptRoot

# BUILD
# & "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\msbuild.exe"
& msbuild $test_prj -t:rebuild -p:Configuration=Debug -p:Platform=x64
if( $lastexitcode -ne 0 )
   {
    Write-Host "Compilation error" -ForegroundColor Red
    Pause
    exit 1
   }


# RUN
Write-Host ""
if( -not (Test-Path $test_exe) )
   {
    Write-Host "$test_exe not generated!" -ForegroundColor Red
    Pause
    exit 1
   }

Write-Host "Launching $test_exe"
& $test_exe
if( $lastexitcode -ne 0 )
   {
    Write-Host "Tests not passed" -ForegroundColor Magenta
    Pause
    exit 1
   }
Write-Host ""
Write-Host "Closing..."
Start-Sleep -Seconds 3
exit 0
