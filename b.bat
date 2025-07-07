@echo off
g++ src/main.cpp src/audio.cpp src/gui.cpp src/globals.cpp -o src/app.exe -std=c++20 ^
-Ilib -Ilib/fftw -Llib/fftw -lfftw3 -lm ^
-Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
run