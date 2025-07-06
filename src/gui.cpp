/*
g++ src/gui.cpp src/globals.cpp -o src/gui.exe -std=c++20 -Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
src/gui.exe
*/
//| INCLUDES ONLY
#include "../lib/raylib/include/raylib.h"
#include "globals.h"

#include <string>
#include <atomic>


void RunGUI() {
    const int WINWIDTH = 1920;
    const int WINHEIGHT = 1080;
    InitWindow(WINWIDTH, WINHEIGHT, "Piano Flashcard App");
    SetTargetFPS(60);
    //| ALL VARIABLES MUST BE DECLARED BELOW
    SetTextLineSpacing(16);
    Font roboto = LoadFontEx("assets/Roboto-Black.ttf", 128, NULL, 0);
    Image staffImage = LoadImage("assets/GrandStaff.png");
    Texture2D grandStaff = LoadTextureFromImage(staffImage);
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        // Draw Image
        DrawTexture(
            grandStaff, 
            (WINWIDTH / 2) - (grandStaff.width / 2),
            (WINHEIGHT / 2) - (grandStaff.height / 2),
            WHITE
        );
        ClearBackground(RAYWHITE);
        DrawTextEx(roboto, currentNote.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
    }
    runningProgram = false;
    UnloadTexture(grandStaff);
    CloseWindow();
}
