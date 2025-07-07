/*
g++ src/gui.cpp src/globals.cpp -o src/gui.exe -std=c++20 -Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
src/gui.exe
*/
//| INCLUDES ONLY
#include <string>
#include <atomic>
#include <iostream>

#include "../lib/raylib/include/raylib.h"
#include "globals.h"

using namespace std;

const float WINWIDTH = 1920;
const float WINHEIGHT = 1080;

// Draws grand staff more easily
void drawStaff(Texture2D txtr) {
    DrawTexture(
        txtr,
        (WINWIDTH / 2) - (txtr.width / 2), 
        (WINHEIGHT / 2) - (txtr.height / 2), 
        WHITE
    );
}
// 40 pixels between each line
// 495 is the bottom line for Treble Clef

// Displays which note is being detected by putting it on the grand staff
void drawNote(Texture2D noteTxtr, Texture2D flatTxtr, Texture2D sharpTxtr, Texture2D naturalTxtr, string noteRaw) {
    string trebleLines[5] = {"E4", "G4", "B4", "D5", "F5"};
    string trebleSpaces[5] = {"D4", "F4", "A4", "C5", "E5"};
    string note = noteRaw.substr(0, 2);
    char accidental = (noteRaw.length() == 3) ? noteRaw.at(1) : '\0';
    int octave = noteRaw.back() - 0;
    if (!(note == "C4" || octave < 4)) {  // Disable ledger lines and base clef for debugging
        ;
    }
}

// Handles all GUI logic
void RunGUI() {
    
    InitWindow(WINWIDTH, WINHEIGHT, "Piano Flashcard App");
    SetTargetFPS(60);
    //| ALL VARIABLES MUST BE DECLARED BELOW
    SetTextLineSpacing(16);
    Font roboto = LoadFontEx("assets/Roboto-Black.ttf", 128, NULL, 0);

    Image tempImage = LoadImage("assets/GrandStaff.png");
    ImageResize(&tempImage, tempImage.width * 2, tempImage.height * 2);
    Texture2D grandStaffTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/sharp.png");
    ImageResize(&tempImage, tempImage.width / 9, tempImage.height / 9);
    Texture2D sharpTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/Flat.png");
    ImageResize(&tempImage, tempImage.width / 9.7, tempImage.height / 9.7);
    Texture2D flatTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/natural.png");
    ImageResize(&tempImage, tempImage.width / 8, tempImage.height / 8);
    Texture2D naturalTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/QuarterNote.png");
    ImageResize(&tempImage, tempImage.width / 1.4, tempImage.height / 1.4);
    Texture2D noteTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawStaff(grandStaffTexture);
        DrawTexture(flatTexture, ((WINWIDTH / 2) - (flatTexture.width / 2)) - 50, (WINHEIGHT / 2) - (flatTexture.height / 2), WHITE);
        DrawTexture(sharpTexture, (WINWIDTH / 2) - (sharpTexture.width / 2), (WINHEIGHT / 2) - (sharpTexture.height / 2), WHITE);
        DrawTexture(naturalTexture, (WINWIDTH / 2) - (naturalTexture.width / 2) + 50, (WINHEIGHT / 2) - (naturalTexture.height / 2), WHITE);
        DrawTexture(noteTexture, (WINWIDTH / 2) - (noteTexture.width / 2) + 100, (WINHEIGHT / 2) - (noteTexture.height / 2), WHITE);

        DrawTextEx(roboto, currentNote.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            cout << "Mouse clicked at: (" << mousePos.x << ", " << mousePos.y << ")\n";
        }
    }
    UnloadTexture(grandStaffTexture);
    UnloadTexture(sharpTexture);
    UnloadTexture(flatTexture);
    UnloadTexture(naturalTexture);
    UnloadTexture(noteTexture);
    runningProgram = false;
    CloseWindow();
}
