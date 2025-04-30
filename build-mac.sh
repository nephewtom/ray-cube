rm -f ./ray-cube

RAYLIB_DIR=../rolling-cube/raylib/src

clang++ -g -std=c++20 -Wall -Wextra -Wno-missing-field-initializers ray-cube.cpp -o ray-cube -I$RAYLIB_DIR  -L$RAYLIB_DIR -lraylib -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit
./ray-cube
