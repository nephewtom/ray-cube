:: Configure emscripten environment in evil-Windows
call c:\emsdk\emsdk_set_env.bat

:: Compile raylib with
:: $ make.exe -f Makefile.Web PLATFORM=PLATFORM_WEB -B

C:\emsdk\upstream\emscripten\emcc.bat -g -o ray-cube.html ../ray-cube.cpp ^
  -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -O3 ^
  -I../../rolling-cube/raylib/src -L../../rolling-cube/raylib/src ^
  -sTOTAL_MEMORY=134217728 ^
  -sUSE_GLFW=3 ^
  -sUSE_WEBGL2=1 ^
  -sUSE_SDL=2 ^
  -sFORCE_FILESYSTEM=1 ^
  -sASYNCIFY ^
  -sEXPORTED_RUNTIME_METHODS=ccall,cwrap ^
  -sMINIFY_HTML=0 ^
  -sASSERTIONS=2 ^
  --shell-file resize.html ^
  --preload-file ../assets ^
  -DPLATFORM_WEB -DRESIZE_CALLBACK ^
  ../../rolling-cube/raylib/src/libraylib.web.a

emrun.bat --browser=chrome ray-cube.html
