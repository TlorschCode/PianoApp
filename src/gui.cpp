// TODO: Add volume slider

//| INIT
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <optional>
#include <cctype>

#include "../lib/raylib/include/raylib.h"
#include "globals.h"

using namespace std;

const float WINWIDTH = 1920;
const float WINHEIGHT = 1080;

static random_device rd;  // seed
static mt19937 gen(rd()); // random number generator

//| ENUMERATORS
enum ButtonType {SQUARE, CIRCLE};
enum MenuState {FLASHCARD_MENU, SETTINGS_MENU};

//| Template functions
// Adds button info to associated lists
// PARAMETERS
// buttonName is a string by which the button will be identified with.
// buttonLocation is where the button is positioned.
// buttonSize is the size of the hitbox of the button.
// buttonType is a string that can either be "circle" or "square" which determines the shape of the hitbox.
void addButton(string buttonName, Vector2 buttonLocation, float buttonSize, string buttonType, Texture2D buttonTexture);
inline string to_lowercase(const string &str);
void mouseLogic(char hoveredBtn, Vector2 toggleBtnLoc, string correctNote); 
void drawNote(string useNote, Color tint);
template <typename T> inline optional<unsigned int> indexOf(const T& itm, const vector<T>& vec);
template <typename T, size_t N> inline optional<unsigned int> indexOf(const T& itm, const T (&arr)[N]);
template <typename T, size_t N> inline constexpr size_t arraySize(T (&)[N]);
template <typename T> inline bool containsVal(const T& itm, const vector<T>& vec);

//| CLASSES
class Button {
    private:
        float ocapacity = 255;
        Color color = {255, 255, 255, 255};
        Rectangle rect = {0, 0, 0, 0};
        Color selection_color = {100, 100, 100, 255};
    public:
        string name;
        Vector2 pos;
        float size;
        ButtonType type;
        Texture2D texture;
        vector<MenuState> available_states;
        bool shown = true;
        bool selected = false;
        Button(string btn_name, Vector2 btn_pos, float btn_size, ButtonType btn_type, Texture2D btn_texture, vector<MenuState> btn_available_states) {
            name = btn_name;
            pos = btn_pos;
            size = btn_size;
            type = btn_type;
            texture = btn_texture;
            available_states = btn_available_states;
            selected = false;
            if (type == SQUARE) {
                rect = {pos.x - size, pos.y - size, size, size};
            }
        }
        void SetOcapacity(float target_amnt) {
            ocapacity = target_amnt;
            color = {255, 255, 255, static_cast<unsigned char>(ocapacity)};
        }
        void AvailableVisibility(MenuState menu_state) {
            shown = containsVal(menu_state, available_states);
        }
        void Render() {
            if (shown) {
                DrawTexture(texture, pos.x - (size), pos.y - (size), color);
                if (selected) {
                    if (type == CIRCLE) {
                        DrawCircle(pos.x, pos.y, size + (size * 0.3), selection_color);
                    }
                }
            }
        }
        inline bool IsHovered(Vector2 mouse_pos) {
            if (type == CIRCLE) {
                return CheckCollisionPointCircle(mouse_pos, pos, size) && shown;
            } else {
                return CheckCollisionPointRec(mouse_pos, rect) && shown;
            }
        }
        inline bool IsLeftClicked(Vector2 mouse_pos) {
            return IsHovered(mouse_pos) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        }
        inline bool IsRightClicked(Vector2 mouse_pos) {
            return IsHovered(mouse_pos) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
        }
        void ToggleSelection() {
            selected = !selected;
        }
        void Hide() {
            shown = false;
        }
        void Show() {
            shown = true;
        }
        void ToggleVisibility() {
            shown = !shown;
        }
        void SetSelectionColor(Color slct_color) {
            selection_color = slct_color;
        }
};

//| Global Variables
// For note detection
vector<Button> buttons = {};
string trebleLines[6] = {"C4", "E4", "G4", "B4", "D5", "F5"};
string trebleSpaces[6] = {"D4", "F4", "A4", "C5", "E5", "G5"};
string baseLines[5] = {"G2", "B2", "D3", "F3", "A3"};
string baseSpaces[6] = {"F2", "A2", "C3", "E3", "G3", "B3"};
string sharpNotes[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
string flatNotes[12] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
int sizeTrebleLines = sizeof(trebleLines) / sizeof(trebleLines[0]);
int sizeTrebleSpaces = sizeof(trebleSpaces) / sizeof(trebleSpaces[0]);
int sizeBaseLines = sizeof(baseLines) / sizeof(baseLines[0]);
int sizeBaseSpaces = sizeof(baseSpaces) / sizeof(baseSpaces[0]);
// Timers and frames
int newNoteTimer = 0;
int checkNoteTimer = 0;
int TIMER = 0;
const int FRAMERATE = 60;
// const int FRAME = floor(1000 / FRAMERATE);
const int FRAME = 16;
MenuState menuState = FLASHCARD_MENU;

//| Template variables
Texture2D grandStaffTexture;
Texture2D sharpTexture;
Texture2D flatTexture;
Texture2D naturalTexture;
Texture2D noteTexture;
Texture2D ledgerTexture;
Texture2D settingsTexture;
Texture2D closeSettingsTexture;
Font roboto;

//| Colors
Color Transparent = {255, 255, 255, 127};
Color sharpCircleCol = GRAY;
Color flatCircleCol = GRAY;

//| FUNCTIONS 
// Returns which index itm is found at
// Returns nullopt if no index was found
//
// itm -> item to search for
// vec -> std::vector to search through
template <typename T>
inline optional<unsigned int> indexOf(const T& itm, const vector<T>& vec) {
    auto it = find(vec.begin(), vec.end(), itm);
    if (it != vec.end()) {
        return static_cast<unsigned int>(it - vec.begin());
    } else {
        return nullopt; // Not found
    }
}

template <typename T>
inline bool containsVal(const T& itm, const vector<T>& vec) {
    auto it = find(vec.begin(), vec.end(), itm);
    if (it != vec.end()) {
        return true;
    } else {
        return false;
    }
}

// Returns which index the item is found at
// Returns nullopt if no index was found
//
// itm -> item to search for
// arr -> C-style array to search through
template <typename T, size_t N>
std::optional<unsigned int> indexOf(const T& itm, const T (&arr)[N]) {
    auto it = std::find(arr, arr + N, itm);
    if (it != arr + N) {
        return static_cast<unsigned int>(it - arr);
    }
    else {
        return nullopt;
    }
}

template <typename T, size_t N>
constexpr size_t arraySize(T (&)[N]) {
    return N;
}

inline int randint(int min, int max) {
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

// Loads assets
void loadAssets() {
    roboto = LoadFontEx("assets/Roboto-Black.ttf", 128, NULL, 0);
    float relativeSizeY = 0;
    float relativeSizeX = 0;
    float standardSize = 100;

    Image tempImage = LoadImage("assets/GrandStaff.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX * 18, relativeSizeY * 18);
    grandStaffTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/sharp.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 4, relativeSizeY / 4);
    sharpTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/Flat.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 4, relativeSizeY / 4);
    flatTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/natural.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 4, relativeSizeY / 4);
    naturalTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/QuarterNote.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 2, relativeSizeY / 2);
    noteTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    // TODO: Scale ledger line correctly
    tempImage = LoadImage("assets/LedgerLine.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX, relativeSizeY);
    ledgerTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/SettingsIcon.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 1.5, relativeSizeY / 1.5);
    settingsTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    tempImage = LoadImage("assets/CloseButton.png");
    relativeSizeX = tempImage.width * (standardSize / tempImage.width);
    relativeSizeY = tempImage.height * (standardSize / tempImage.width);
    ImageResize(&tempImage, relativeSizeX / 1.5, relativeSizeY / 1.5);
    closeSettingsTexture = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);
}

// Wait in miliseconds
void wait(int time) {
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

// Changes timers and wait 1/60th of a second.
void tick() {
    wait(FRAME);
    TIMER += 1;
    TIMER = TIMER == 60 ? 0 : TIMER;
    newNoteTimer -= 1;
    newNoteTimer = newNoteTimer == -1 ? 0 : newNoteTimer;
    checkNoteTimer -= 1;
    checkNoteTimer = checkNoteTimer == -1 ? 0 : checkNoteTimer;
}

// Quality of life function to display text.
// Allows for custom font size.
void displayText(const char *text, float x, float y, float fontSize, Color color) {
    DrawTextEx(roboto, text, {x, y}, fontSize, 2, color);
}

// Quality of life function to display text.
void displayText(const char *text, float x, float y, Color color) {
    DrawTextEx(roboto, text, {x, y}, 30, 2, color);
}

// Quality of life function to draw the Grand Staff.
void drawStaff(Texture2D txtr) {
    DrawTexture(
        txtr,
        (WINWIDTH / 2) - (txtr.width / 2), 
        (WINHEIGHT / 2) - (txtr.height / 2), 
        WHITE
    );
}

// Displays whichever note is passed into it.
// The tint of the textures is changeable, but defaults to White.
void drawNote(string useNote, Color tint = WHITE) {
    // 44.5 pixels between each line
    // 495 is the bottom line for Treble Clef, or E4
    // 803 is the bottom line for Base Clef, or G2
    int y = 0;
    int x = 500;
    float noteStep = 44.5;
    int accidentalXOffset = -30;
    int accidentalYOffset = -5;
    int ledgerXOffset = 0;
    int noteYOffset = 45;

    //# Note Decoding
    string note = (useNote.length() == 3) ? (useNote.substr(0, 1) + useNote.back()) : useNote;
    char accidental = (useNote.length() == 3) ? useNote.at(1) : '\0';
    bool isTrebleLine = indexOf(note, trebleLines) != nullopt;
    bool isTrebleSpace = indexOf(note, trebleSpaces) != nullopt;
    bool isTreble = isTrebleLine || isTrebleSpace;
    bool isBaseLine = indexOf(note, baseLines) != nullopt;
    bool isBaseSpace = indexOf(note, baseSpaces) != nullopt;
    bool isBase = isBaseLine || isBaseSpace;

    //# Note Rendering
    if (isTreble) {
        y = isTrebleLine ? 538 - (noteStep * indexOf(note, trebleLines).value_or(-1)) : 513 - (noteStep * indexOf(note, trebleSpaces).value_or(-1));
        if (note == "C4") {
            DrawTexture(ledgerTexture, x + ledgerXOffset, y - (ledgerTexture.height / 2), tint);
        }
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + accidentalXOffset, y - ((sharpTexture.height / 2) + accidentalYOffset), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + accidentalXOffset, y - ((flatTexture.height / 2) + accidentalYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + accidentalXOffset,  y - ((naturalTexture.height / 2) + accidentalYOffset), tint);
                    break;
            }
        }
    } else if (isBase) {  //# Base Clef
        y = isBaseLine ? 800 - (noteStep * indexOf(note, baseLines).value_or(-1)) :  822 - (noteStep * indexOf(note, baseSpaces).value_or(-1));
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + accidentalXOffset, y - ((sharpTexture.height / 2) + accidentalYOffset), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + accidentalXOffset, y - ((flatTexture.height / 2) + accidentalYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + accidentalXOffset,  y - ((naturalTexture.height / 2) + accidentalYOffset), tint);
                    break;
            }
        }
    }
}

// Checks whether the correct note is being played.
// Handles new target note logic and renders the target note on the screen.
void checkNote(string *correctNote) {
    char octave = (*correctNote).back();
    char accidental = (*correctNote).length() == 3 ? (*correctNote).at(1) : '\0';
    string note = (*correctNote).substr(0, 1);
    if (SHARPS) {
        if (accidental == 'b') {
            cout << "  DEBUG:  " << note + accidental << "\n";
            int index = indexOf((note + accidental), flatNotes).value_or(-1);
            cout << "  DEBUG:  " << index << "\n";
            cout << "  DEBUG:  " << sharpNotes[index] << "\n";
            *correctNote = sharpNotes[index] + octave;
        }
    } else {
        if (accidental == '#') {
            cout << "  DEBUG:  " << note + accidental << "\n";
            int index = indexOf((note + accidental), sharpNotes).value_or(-1);
            cout << "  DEBUG:  " << index << "\n";
            cout << "  DEBUG:  " << flatNotes[index];
            *correctNote = flatNotes[index] + octave;
        }
    }
    if (CURRENTNOTE == *correctNote) {
        if (checkNoteTimer == 1) {
            newNoteTimer = 15;
        } else if (checkNoteTimer == 0) {
            checkNoteTimer = 6; // Makes sure the current note stays for a second after reaching it.
        }
    }
    if (newNoteTimer > 0) {
        drawNote(*correctNote);
    }
    if (newNoteTimer == 1) {
        if (randint(0, 1)) {
            if (randint(0, 1)) {
                *correctNote = trebleLines[(randint(0, sizeTrebleLines - 1))];
            } else {
                *correctNote = trebleSpaces[(randint(0, sizeTrebleSpaces - 1))];
            }
        } else {
            if (randint(0, 1)) {
                *correctNote = baseSpaces[(randint(0, sizeBaseSpaces - 1))];
            } else {
                *correctNote = baseLines[(randint(0, sizeBaseLines - 1))];
            }
        }
        if (randint(0, 1)) {
            if (SHARPS) {
                if ((*correctNote).substr(0,1) == "E" ) {
                    *correctNote = "F" + (*correctNote).substr(1,1);
                } else if ((*correctNote).substr(0,1) == "B") {
                    *correctNote = "C" + (*correctNote).substr(1,1);
                } else {
                    *correctNote = (*correctNote).substr(0,1) + "#" + (*correctNote).substr(1,1);
                }
            } else {
                if ((*correctNote).substr(0,1) == "F" ) {
                    *correctNote = "E" + (*correctNote).substr(1,1);
                } else if ((*correctNote).substr(0,1) == "C") {
                    *correctNote = "B" + (*correctNote).substr(1,1);
                } else {
                    *correctNote = (*correctNote).substr(0,1) + "b" + (*correctNote).substr(1,1);
                }
            }
        }
    }
}

void drawButtons() {
    bool should_render = true;
    for (Button &button : buttons) {
        button.AvailableVisibility(menuState);
        button.Render();
    }
}

bool mouseLogic(string *hovered) {
    *hovered = "???";
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        cout << ">>>> DEBUG: " << mouse.x << ", " << mouse.y << "\n";
    }
    for (Button &button : buttons) {
        if (button.IsHovered(mouse)) {
            *hovered = button.name;
        }
        if (button.IsLeftClicked(mouse)) {
            return true;
        }
    }
    return false;
}

void buttonLogic(bool clicked, string hovered_btn) {
    if (clicked) {
        if (hovered_btn == "settings") {
            menuState = SETTINGS_MENU;
        } else if (hovered_btn == "close_settings") {
            menuState = FLASHCARD_MENU;
        } else if (hovered_btn == "do_sharps") {
            SHARPS = true;
        }
    }
}

void flashcardMenu(string crct_note) {
    checkNote(&crct_note);
    drawStaff(grandStaffTexture);
    drawNote(CURRENTNOTE, Transparent);
    drawNote(crct_note, Color {255, 255, 255, 75});
}

void settingsMenu() {
    drawStaff(grandStaffTexture);
    drawNote(CURRENTNOTE, Transparent);
    drawNote("C#4", Color {255, 255, 255, 255});
}

// Handles all GUI logic
void RunGUI() {
    InitWindow(WINWIDTH, WINHEIGHT, "Piano Flashcard App");
    SetTargetFPS(60);
    SetTextLineSpacing(16);
    loadAssets();

    Button settingsButton("settings", Vector2({WINWIDTH - 40, 80}), 50, CIRCLE, settingsTexture, vector<MenuState> {FLASHCARD_MENU});
    buttons.push_back(settingsButton);
    Button closeSettingsButton("close_settings", Vector2({WINWIDTH - 40, 80}), 50, CIRCLE, closeSettingsTexture, vector<MenuState> {SETTINGS_MENU});
    buttons.push_back(closeSettingsButton);
    // TODO: Make this an actual button, not just a texture of a flat.
    Button sharpButton("do_sharps", Vector2({WINWIDTH - 500, 90}), 50, CIRCLE, sharpTexture, vector<MenuState> {SETTINGS_MENU});
    buttons.push_back(sharpButton);

    string hoveredBtn = "???";
    newNoteTimer = 0;
    string correctNote = "G#2";
    bool was_clicked = false;

    while (!WindowShouldClose()) {
        buttonLogic(mouseLogic(&hoveredBtn), hoveredBtn);
        ClearBackground(RAYWHITE);
        drawButtons();
        if (menuState == FLASHCARD_MENU) {
            flashcardMenu(correctNote);
        } else if (menuState == SETTINGS_MENU) {
            settingsMenu();
        }
        DrawTextEx(roboto, CURRENTNOTE.c_str(), {100, 100}, 40, 2, DARKGRAY);
        EndDrawing();
        tick();
    }
    UnloadTexture(grandStaffTexture);
    UnloadTexture(sharpTexture);
    UnloadTexture(flatTexture);
    UnloadTexture(naturalTexture);
    UnloadTexture(noteTexture);
    UnloadTexture(ledgerTexture);
    UnloadTexture(settingsTexture);
    UnloadFont(roboto);
    RUNNINGPROGRAM = false;
    CloseWindow();
}
