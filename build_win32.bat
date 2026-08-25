@echo off

:: Use PowerShell to safely define a real Escape character (ASCII 27)
for /f "delims=" %%A in ('powershell -Command "[char]27"') do set "ESC=%%A"
:: Define the pastel green and reset sequences
set "PASTEL_RED=%ESC%[38;2;220;120;120m"
set "PASTEL_GREEN=%ESC%[38;2;120;200;140m"
set "RESET=%ESC%[0m"

REM get the cl compiler setup
REM if not defined DevEnvDir (
REM     call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat" x64
REM )

if not exist ".\build\debug" mkdir ".\build\debug"


REM CL compiler build command
REM cl src\win32_main.cpp /Zi /Fo:build\debug\ /Fe:build\debug\win32_d3d11_main.exe /link /SUBSYSTEM:WINDOWS user32.lib

REM Clang complier build command
clang++ src\win32_main.cpp -o build\debug\win32_d3d11_main.exe -g -Xlinker /subsystem:windows -luser32


if errorlevel 1 (
    echo Compilation %PASTEL_RED%failed%RESET%
    exit /b 1
)

echo Compilation finished %PASTEL_GREEN%successfully%RESET%
