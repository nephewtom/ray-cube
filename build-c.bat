@echo off
del .\ray-cube.exe

set RAYLIB_DIR=..\rolling-cube\raylib\src
@echo on
gcc -g -Wall ray-cube.c -o ray-cube.exe -I%RAYLIB_DIR% -L%RAYLIB_DIR% -lraylib -lgdi32 -lwinmm
@echo off
.\ray-cube.exe
