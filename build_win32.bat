@echo off

mkdir ".\build\debug"
clang++ main.cpp -o build\debug\main.exe -g -Xlinker /subsystem:windows
