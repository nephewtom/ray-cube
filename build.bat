@echo off
del .\ray-cube.exe

set RAYLIB_DIR=..\rolling-cube\raylib\src
@echo on
g++ -g -Wall ray-cube.cpp -o ray-cube.exe -I%RAYLIB_DIR% -L%RAYLIB_DIR% -lraylib -lgdi32 -lwinmm -std=c++20
@echo off
.\ray-cube.exe
