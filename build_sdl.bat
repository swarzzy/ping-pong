@echo off
cmake -Sext/SDL2-2.0.12 -Bbuild/SDL -G"Visual Studio 15 2017 Win64" -DSDL_STATIC=ON -DSDL_SHARED=OFF -DFORCE_STATIC_VCRT=ON
cmake --build build/SDL
copy /Y build\SDL\Debug\SDL2d.lib build\SDL2d.lib
rem msbuild build\SDL\SDL2.sln /nologo /property:Configuration=Debug /property:Platform=x64 /maxCpuCount:4 /verbosity:quiet
