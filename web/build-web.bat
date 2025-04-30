:: Configure emscripten environment
:: c:\emsdk\emsdk_set_env.bat
call c:\emsdk\emsdk_set_env.bat

:: Compile raylib with
:: $ make.exe -f Makefile.Web PLATFORM=PLATFORM_WEB -B


C:\emsdk\upstream\emscripten\emcc.bat -g -o ray-cube.html ..\ray-cube.c -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -O3 -I../../rolling-cube/raylib/src -L../../rolling-cube/raylib/src -sTOTAL_MEMORY=134217728 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sFORCE_FILESYSTEM=1 -sMINIFY_HTML=0 -sUSE_GLFW=3 -s ASYNCIFY ../../rolling-cube/raylib/src/libraylib.web.a -DPLATFORM_WEB -s FULL_ES3=1 -sASSERTIONS=1 --shell-file shell.html --preload-file ../assets
:: -s USE_WEBGL2=1
