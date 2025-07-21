//| INIT
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#include "../lib/raylib/include/raylib.h"
#include "globals.h"

using namespace std;

const float WINWIDTH = 1920;
const float WINHEIGHT = 1080;

//| Global Variables
// For note detection
string trebleLines[6] = {"C4", "E4", "G4", "B4", "D5", "F5"};
string trebleSpaces[6] = {"D4", "F4", "A4", "C5", "E5", "G5"};
string baseLines[5] = {"G2", "B2", "D3", "F3", "A3"};
string baseSpaces[6] = {"F2", "A2", "C3", "E3", "G3", "B3"};
int sizeTrebleLines = sizeof(trebleLines) / sizeof(trebleLines[0]);
int sizeTrebleSpaces = sizeof(trebleSpaces) / sizeof(trebleSpaces[0]);
int sizeBaseLines = sizeof(baseLines) / sizeof(baseLines[0]);
int sizeBaseSpaces = sizeof(baseSpaces) / sizeof(baseSpaces[0]);
// For note rendering
float noteStep = 40.9;
int sharpXOffset = -26;
int flatXOffset = -9;
int flatYOffset = 15;
int naturalXOffset = -20;
int naturalYOfffset = 45;
int ledgerXOffset = 28;
int noteYOffset = 45;
int newNoteTimer = 0;
int TIMER = 0;
const int FRAMERATE = 60;
const int FRAME(1000 / FRAMERATE);

//| Template variables
Texture2D grandStaffTexture;
Texture2D sharpTexture;
Texture2D flatTexture;
Texture2D naturalTexture;
Texture2D noteTexture;
Texture2D ledgerTexture;
Font roboto;

//| Template functions
void mouseLogic(char hoveredBtn, Vector2 toggleBtnLoc); 
void drawNote(string useNote, Color tint);

//| Colors
Color Transparent = {255, 255, 255, 127};
Color sharpCircleCol = GRAY;
Color flatCircleCol = GRAY;

//| FUNCTIONS
void loadAssets() {
    roboto = LoadFontEx("assets/Roboto-Black.ttf", 128, NULL, 0);

    Image tempImage = LoadImage("assets/GrandStaff.png");
    ImageResize(&tempImage, tempImage.width * 2, tempImage.height * 2);
    grandStaffTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/sharp.png");
    ImageResize(&tempImage, tempImage.width / 9, tempImage.height / 9);
    sharpTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/Flat.png");
    ImageResize(&tempImage, tempImage.width / 9.7, tempImage.height / 9.7);
    flatTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/natural.png");
    ImageResize(&tempImage, tempImage.width / 8, tempImage.height / 8);
    naturalTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/QuarterNote.png");
    ImageResize(&tempImage, tempImage.width / 1.4, tempImage.height / 1.4);
    noteTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/LedgerLine.png");
    ImageResize(&tempImage, tempImage.width / 2.5, tempImage.height / 2);
    ledgerTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);
}

// Wait in miliseconds
void wait(int time) {
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

// Changes timers and wait 1/60th of a second
void tick() {
    wait(FRAME);
    TIMER += 1;
    TIMER = TIMER == 60 ? 0 : TIMER;
    newNoteTimer -= 1;
    newNoteTimer = newNoteTimer == -1 ? 0 : newNoteTimer;
}
void displayText(const char *text, float x, float y, float fontSize, Color color) {
    DrawTextEx(roboto, text, {x, y}, fontSize, 2, color);
}

void displayText(const char *text, float x, float y, Color color) {
    DrawTextEx(roboto, text, {x, y}, 30, 2, color);
}

// Draws grand staff more easily
void drawStaff(Texture2D txtr) {
    DrawTexture(
        txtr,
        (WINWIDTH / 2) - (txtr.width / 2), 
        (WINHEIGHT / 2) - (txtr.height / 2), 
        WHITE
    );
}

// Displays whichever note is passed into it.
// The tint of the textures is changeable, but defaults to White
void drawNote(string useNote, Color tint = WHITE) {
    // 40.9 pixels between each line
    // 495 is the bottom line for Treble Clef, or E4
    // 785 is the bottom line for Base Clef, or G2
    //# Note Decoding
    int y = 0;
    int x = 500;

    string note = (useNote.length() == 3) ? (useNote.substr(0, 1) + useNote.back()) : useNote;
    char accidental = (useNote.length() == 3) ? useNote.at(1) : '\0';
    int octave = useNote.back() - '0';
    bool isTrebleLine = find(begin(trebleLines), end(trebleLines), note) != end(trebleLines);
    bool isTrebleSpace = find(begin(trebleSpaces), end(trebleSpaces), note) != end(trebleSpaces);
    bool isTreble = isTrebleLine || isTrebleSpace;
    bool isBaseLine = find(begin(baseLines), end(baseLines), note) != end(baseLines);
    bool isBaseSpace = find(begin(baseSpaces), end(baseSpaces), note) != end(baseSpaces);
    bool isBase = isBaseLine || isBaseSpace;

    //# Note Rendering
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
            DrawTexture(ledgerTexture, x + ledgerXOffset, y - (ledgerTexture.height / 2), tint);
        }
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + sharpXOffset, y - (sharpTexture.height / 2), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + flatXOffset, y - ((flatTexture.height / 2) + flatYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + naturalXOffset,  y - ((naturalTexture.height / 2) + naturalYOfffset), tint);
                    break;
            }
        }
    } else if (isBase) {  //# Base Clef
        if (isBaseLine) {
            auto search = find(baseLines, baseLines + sizeBaseLines, note);
            int index = search - baseLines;
            y = 785 - (noteStep * index);
        } else {
            auto search = find(baseSpaces, baseSpaces + sizeBaseSpaces, note);
            int index = search - baseSpaces;
            y = 805 - (noteStep * index);
        }
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + sharpXOffset, y - (sharpTexture.height / 2), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + flatXOffset, y - ((flatTexture.height / 2) + flatYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + naturalXOffset,  y - ((naturalTexture.height / 2) + naturalYOfffset), tint);
                    break;
            }
        }
    }
}

void checkNote(string *correctNote) {
    if (CURRENTNOTE == *correctNote) {
        drawNote(*correctNote);
        newNoteTimer = 30;
    }
}

void drawSharpToggle(float x, float y, char hoveredButton, Color *sharpCol, Color *flatCol) {
    Color colorHoverGray = {150, 150, 150, 255};
    float hoverGray = 150.0f;
    DrawRectangle(x - 20, y, 40, 40, GRAY);
    DrawCircle(x, y, 20, GRAY);
    DrawCircle(x, y + 40, 20, GRAY);

    float mainSharpCol = (float)(sharpCol->r);
    float mainFlatCol = (float)(flatCol->r);
    Color betweenSharpCol = *sharpCol;
    Color betweenFlatCol = *flatCol;
    float interpolationSpeed = 0.3;
    if (hoveredButton == '#') {
        unsigned char newCol = (unsigned char)(mainSharpCol + ((hoverGray - mainSharpCol) * interpolationSpeed));
        betweenSharpCol = {newCol, newCol, newCol, 255};
        *sharpCol = betweenSharpCol;
        newCol = (unsigned char)(mainFlatCol + ((GRAY.r - mainFlatCol) * interpolationSpeed));
        betweenFlatCol = {newCol, newCol, newCol, 255};
        *flatCol = betweenFlatCol;
    } else if (hoveredButton == 'b') {
        unsigned char newCol = (unsigned char)(mainFlatCol + ((hoverGray - mainFlatCol) * interpolationSpeed));
        betweenFlatCol = {newCol, newCol, newCol, 255};
        *flatCol = betweenFlatCol;
        newCol = (unsigned char)(mainSharpCol + ((GRAY.r - mainSharpCol) * interpolationSpeed));
        betweenSharpCol = {newCol, newCol, newCol, 255};
        *sharpCol = betweenSharpCol;
    } else {
        unsigned char newCol = (unsigned char)(mainSharpCol + ((GRAY.r - mainSharpCol) * interpolationSpeed));
        betweenSharpCol = {newCol, newCol, newCol, 255};
        *sharpCol = betweenSharpCol;
        newCol = (unsigned char)(mainFlatCol + ((GRAY.r - mainFlatCol) * interpolationSpeed));
        betweenFlatCol = {newCol, newCol, newCol, 255};
        *flatCol = betweenFlatCol;
    }
    DrawCircle(x, y, 18, betweenSharpCol);
    DrawCircle(x, y + 39, 18, betweenFlatCol);
    if (SHARPS) {
        DrawRing({x, y}, 15, 18, 0, 360, 360, DARKGRAY);
    } else {
        DrawRing({x, y + 40}, 15, 18, 0, 360, 360, DARKGRAY);
    }
    displayText("#", x - 6, y - 14, BLACK);
    displayText("b", x - 6, y + 24, BLACK);
}

void mouseLogic(char hoveredBtn, Vector2 toggleBtnLoc) {
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointCircle(mouse, toggleBtnLoc, 20)) {
        hoveredBtn = '#';
    } else if (CheckCollisionPointCircle(mouse, {toggleBtnLoc.x, toggleBtnLoc.y + 40}, 20)) {
        hoveredBtn = 'b';
    } else {
        hoveredBtn = 'X';
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointCircle(mouse, toggleBtnLoc, 20)) {
            SHARPS = true;
        } else if (CheckCollisionPointCircle(mouse, {toggleBtnLoc.x, toggleBtnLoc.y + 40}, 20)) {
            SHARPS = false;
        }
    }
}

// Handles all GUI logic
void RunGUI() {
    InitWindow(WINWIDTH, WINHEIGHT, "Piano Flashcard App");
    SetTargetFPS(60);
    SetTextLineSpacing(16);
    loadAssets();

    Vector2 toggleLoc = {WINWIDTH - 100, 100};
    char hovered = 'X';
    newNoteTimer = 0;
    string correctNote = "F3";

    while (!WindowShouldClose()) {
        mouseLogic(hovered, toggleLoc);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        checkNote(&correctNote);
        drawStaff(grandStaffTexture);
        drawNote(CURRENTNOTE, Transparent);
        drawSharpToggle(WINWIDTH - 100, 100, hovered, &sharpCircleCol, &flatCircleCol);

        DrawTextEx(roboto, CURRENTNOTE.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
        tick();
    }
    UnloadTexture(grandStaffTexture);
    UnloadTexture(sharpTexture);
    UnloadTexture(flatTexture);
    UnloadTexture(naturalTexture);
    UnloadTexture(noteTexture);
    UnloadFont(roboto);
    RUNNINGPROGRAM = false;
    CloseWindow();
}
