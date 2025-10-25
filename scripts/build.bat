@echo off
setlocal enableextensions
setlocal enabledelayedexpansion

rem Search for MSYS2 base installation
set "POSSIBLE_DIRS="%ProgramFiles%\msys64" "%ProgramFiles(x86)%\msys64" "C:\msys64" "D:\msys64" "%USERPROFILE%\msys64""

set "MSYS64_DIR="

for %%D in (%POSSIBLE_DIRS%) do (
    set "DIR=%%~D"
    if exist "!DIR!\mingw64\bin\g++.exe" (
        set "MSYS64_DIR=!DIR!"
        goto :found
    )
)

echo [ERROR] Could not find MSYS2/mingw64 installation.
echo Tried the following directories:
for %%D in (%POSSIBLE_DIRS%) do echo    %%~D
exit /b 1

:found
set "CXX=%MSYS64_DIR%\mingw64\bin\g++.exe"
set "PKG_CONFIG_PATH=%MSYS64_DIR%\mingw64\lib\pkgconfig"

if "%1"=="clean" (
    make -f scripts\Makefile clean
    exit /b !ERRORLEVEL!
)

echo [INFO] Found MSYS2 at: %MSYS64_DIR%

rem Add MinGW64 binaries to PATH
set "PATH=%MSYS64_DIR%\mingw64\bin;%PATH%"

echo [INFO] Environment configured:
echo    CXX=%CXX%
echo    PKG_CONFIG_PATH=%PKG_CONFIG_PATH%

echo [INFO] Running build system...
make -f scripts\Makefile
set MAKE_EXIT_CODE=!ERRORLEVEL!
if !MAKE_EXIT_CODE! neq 0 (
    echo [INFO] Build failed...    
) else (
    echo [INFO] Build succeeded...
)

