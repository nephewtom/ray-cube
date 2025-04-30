rm -f ./cube
clang++ -g -std=c++20 -Wall -Wextra -Wno-missing-field-initializers src/log.cpp src/main.cpp -o cube -I./src -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -framework OpenGL -framework CoreFoundation -framework CoreGraphics -framework IOKit -framework AppKit
./cube
