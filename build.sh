rm -f ./cube
g++ -g  -std=c++17 -Wall src/main.cpp src/log.cpp -o cube -I./src -I./rlImGui/src -I ./rlImGui/imgui -I ./rlImGui/imgui/backends -I ./raylib/src -L./rlImGui/src -L./raylib/src -lrlImGui -lraylib -Wall -Wextra -Wno-missing-field-initializers
./cube
