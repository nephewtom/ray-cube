rm -f ./ray-cube

RAYLIB_DIR=../raylib/src

g++ -g  -std=c++17 -Wall ray-cube.cpp -o ray-cube -I$RAYLIB_DIR  -L$RAYLIB_DIR -lraylib -lm -ldl -lpthread -Wall -Wextra -Wno-missing-field-initializers
./ray-cube
