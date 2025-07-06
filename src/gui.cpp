/*
g++ src/gui.cpp src/globals.cpp -o src/gui.exe -std=c++20 -Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
src/gui.exe
*/
//| INCLUDES ONLY
#include "../lib/raylib/include/raylib.h"
#include "globals.h"

#include <string>


void RunGUI() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Piano Flashcard App");
    SetTargetFPS(60);
    //| ALL VARIABLES MUST BE DECLARED BELOW
    SetTextLineSpacing(16);
    Font roboto = LoadFontEx("src/assets/Roboto-Black.ttf", 128, NULL, 0);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTextEx(roboto, currentNote.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
}
