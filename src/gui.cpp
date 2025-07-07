/*
g++ src/gui.cpp src/globals.cpp -o src/gui.exe -std=c++20 -Ilib/raylib/include -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static
src/gui.exe
*/
//| INCLUDES ONLY
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm>

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

// Displays which note is being detected by putting it on the grand staff
void drawNote(Texture2D noteTxtr, Texture2D sharpTxtr, Texture2D flatTxtr, Texture2D naturalTxtr, Texture2D ledgerTxtr, string noteRaw) {
    // 40.9 pixels between each line
    // 495 is the bottom line for Treble Clef, or E4
    // 785 is the bottom line for Base Clef, or G2
    string trebleLines[6] = {"C4", "E4", "G4", "B4", "D5", "F5"};
    int sizeTrebleLines = sizeof(trebleLines) / sizeof(trebleLines[0]);
    string trebleSpaces[6] = {"D4", "F4", "A4", "C5", "E5", "G5"};
    int sizeTrebleSpaces = sizeof(trebleSpaces) / sizeof(trebleSpaces[0]);

    string baseLines[5] = {"G2", "B2", "D3", "F3", "A3"};
    int sizeBaseLines = sizeof(baseLines) / sizeof(baseLines[0]);
    string baseSpaces[6] = {"F2", "A2", "C3", "E3", "G3", "B3"};
    int sizeBaseSpaces = sizeof(baseSpaces) / sizeof(baseSpaces[0]);

    string note = (noteRaw.length() == 3) ? (noteRaw.substr(0, 1) + noteRaw.back()) : noteRaw;
    char accidental = (noteRaw.length() == 3) ? noteRaw.at(1) : '\0';
    int octave = noteRaw.back() - '0';
    bool isTrebleLine = find(begin(trebleLines), end(trebleLines), note) != end(trebleLines);
    bool isTrebleSpace = find(begin(trebleSpaces), end(trebleSpaces), note) != end(trebleSpaces);
    bool isTreble = isTrebleLine || isTrebleSpace;

    bool isBaseLine = find(begin(baseLines), end(baseLines), note) != end(baseLines);
    bool isBaseSpace = find(begin(baseSpaces), end(baseSpaces), note) != end(baseSpaces);
    bool isBase = isBaseLine || isBaseSpace;

    int y = 0;
    int x = 500;
    float noteStep = 40.9;

    int sharpXOffset = -26;
    int flatXOffset = -9;
    int flatYOffset = 15;
    int naturalXOffset = -20;
    int naturalYOfffset = 45;
    int ledgerXOffset = 28;

    int noteYOffset = 45;

    if (isTreble) {
        if (isTrebleLine) {
            auto search = find(trebleLines, trebleLines + sizeTrebleLines, note);
            int index = search - trebleLines;
            y = 535 - (noteStep * index);
        } else {
            auto search = find(trebleSpaces, trebleSpaces + sizeTrebleSpaces, note);
            int index = search - trebleSpaces;
            y = 515 - (noteStep * index);
        }
        if (note == "C4") {
            DrawTexture(ledgerTxtr, x + ledgerXOffset, y - (ledgerTxtr.height / 2), WHITE);
        }
        DrawTexture(noteTxtr, x, y - ((noteTxtr.height / 2) + noteYOffset), WHITE);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTxtr, x + sharpXOffset, y - (sharpTxtr.height / 2), WHITE);
                    break;
                case 'b':
                    DrawTexture(flatTxtr, x + flatXOffset, y - ((flatTxtr.height / 2) + flatYOffset), WHITE);
                    break;
                case 'N':
                    DrawTexture(naturalTxtr, x + naturalXOffset,  y - ((naturalTxtr.height / 2) + naturalYOfffset), WHITE);
                    break;
            }
        }
    } else if (isBase) {
        if (isBaseLine) {
            auto search = find(baseLines, baseLines + sizeBaseLines, note);
            int index = search - baseLines;
            y = 785 - (noteStep * index);
        } else {
            auto search = find(baseSpaces, baseSpaces + sizeBaseSpaces, note);
            int index = search - baseSpaces;
            y = 805 - (noteStep * index);
        }
        DrawTexture(noteTxtr, x, y - ((noteTxtr.height / 2) + noteYOffset), WHITE);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTxtr, x + sharpXOffset, y - (sharpTxtr.height / 2), WHITE);
                    break;
                case 'b':
                    DrawTexture(flatTxtr, x + flatXOffset, y - ((flatTxtr.height / 2) + flatYOffset), WHITE);
                    break;
                case 'N':
                    DrawTexture(naturalTxtr, x + naturalXOffset,  y - ((naturalTxtr.height / 2) + naturalYOfffset), WHITE);
                    break;
            }
        }
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

    tempImage = LoadImage("assets/LedgerLine.png");
    ImageResize(&tempImage, tempImage.width / 2.5, tempImage.height / 2);
    Texture2D ledgerTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    Rectangle toggleArea = {WINWIDTH - 250, WINHEIGHT - 100, 100, 50};

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, toggleArea)) {
                SHARPS = !SHARPS;
            }
            std::cout << "Mouse Clicked at (" << mouse.x << ", " << mouse.y << ")\n";
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawStaff(grandStaffTexture);
        drawNote(noteTexture, sharpTexture, flatTexture, naturalTexture, ledgerTexture, CURRENTNOTE);
        
        // FLAT TOGGLE
        DrawRectangleRec(toggleArea, SHARPS ? GREEN : GRAY);
        // Draw sliding circle
        float circleX = toggleArea.x + (SHARPS ? toggleArea.width - toggleArea.height : 0);
        float circleY = toggleArea.y;
        DrawCircleV({ circleX + toggleArea.height / 2, circleY + toggleArea.height / 2 }, toggleArea.height / 2 - 4, WHITE);
        // Label
        DrawTextEx(roboto, SHARPS ? "SHARPS" : "FLATS", {toggleArea.x + 110, toggleArea.y + 10}, 35, 2, BLACK);

        DrawTextEx(roboto, CURRENTNOTE.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
    }
    UnloadTexture(grandStaffTexture);
    UnloadTexture(sharpTexture);
    UnloadTexture(flatTexture);
    UnloadTexture(naturalTexture);
    UnloadTexture(noteTexture);
    RUNNINGPROGRAM = false;
    CloseWindow();
}
