rm -f ./ray-cube

RAYLIB_DIR=../rolling-cube/raylib/src

g++ -g  -std=c++17 -Wall ray-cube.cpp -o ray-cube -I$RAYLIB_DIR  -L$RAYLIB_DIR -lraylib -Wall -Wextra -Wno-missing-field-initializers
./ray-cube
