@echo off

:: Use PowerShell to safely define a real Escape character (ASCII 27)
for /f "delims=" %%A in ('powershell -Command "[char]27"') do set "ESC=%%A"
:: Define the pastel green and reset sequences
set "PASTEL_RED=%ESC%[38;2;220;120;120m"
set "PASTEL_GREEN=%ESC%[38;2;120;200;140m"
set "RESET=%ESC%[0m"


:: *SLOW set up MSVC environment (cl, link, etc.) if not already set up NOTE: x64 is to specify we want the 64bits compiler
:: if not defined DevEnvDir (
::    call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat" x64
:: )

if not exist ".\build\debug" mkdir ".\build\debug"

:: compile with cl i.e. msvc compiler so its fully compatible with visual studio debugger
:: cl src\win32_main.cpp /Zi /Fo:build\debug\ /Fe:build\debug\win32_d3d11_main.exe /link /SUBSYSTEM:WINDOWS user32.lib

:: TODO(harsh): pass a -debug / -release flag to this batch file, to figure out which build command to run
:: NOTE(harsh): Clang complier DEBUG build command
:: (this is called a unity build, because we only have one translation unit i.e. win32_main.cpp)
clang++ src\win32\win32_platform.cpp -o build\debug\win32_d3d11_main.exe -g -DISEKAIED_DEBUG -Xlinker /subsystem:windows -luser32 -ld3d11 -ld3dcompiler



if errorlevel 1 (
    echo Compilation %PASTEL_RED%failed%RESET%
    exit /b 1
)

echo Compilation finished %PASTEL_GREEN%successfully%RESET%
