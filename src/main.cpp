/*
g++ src/main.cpp src/audio.cpp src/gui.cpp src/globals.cpp -o src/app.exe -std=c++20 -Ilib -Ilib/fftw -Llib/fftw -lfftw3 -lm -Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
src/app.exe
*/
#include <iostream>
#include "audio.h"
#include "gui.h"
#include <thread>

int main() {
    std::cout << "Starting App\n";

    std::cout << "Running Audio";
    std::thread audioThread(InitAudio);

    std::cout << "Running GUI";
    std::thread guiThread(RunGUI);

    audioThread.join();
    guiThread.join();

    return 0;
}
