#ifdef DEBUG
    #include <iostream>
    #define DEBUG_LOG(x) cout << "  DEBUG:  " << x << "\n"
#else
    #define DEBUG_LOG(x)
#endif

// MARK: Init
//| INIT  vvv
#include <string>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <optional>
#include <cctype>
#include <fstream>

#include "../lib/raylib/include/raylib.h"
#include "globals.h"
#include "notes.hpp"

using namespace std;
//| END INIT ^^^

// TODO: Add volume slider
// TODO: Add settings option to disable accidentals
// TODO: Make ledger line toggle in settings menu
// TODO: Make base/treble clef toggle in settings menu

// MARK: Constants
//| DEFINITIONS AND CONSTANTS
#define TREBLE_CLEF   "\xF0\x9D\x84\x9E" // U+1D11E 𝄞
#define BASS_CLEF     "\xF0\x9D\x84\xA2" // U+1D122 𝄢
#define SYSTEM_BRACE  "\xEE\x80\x80"     // U+E000 (Bravura PUA)

const int FRAMERATE = {60};
// const int FRAME = floor(1000 / FRAMERATE);
const int FRAME = {16};
const char* ASSET_PATH = {"src/assets/"};

const float WINWIDTH = 1920;
const float WINHEIGHT = 1080;

static random_device rd;  // seed
static mt19937 gen(rd()); // random number generator

//| ENUMERATORS
enum ButtonType {SQUARE, CIRCLE};
enum MenuState {FLASHCARD_MENU, SETTINGS_MENU};

//| Template functions
inline string to_lowercase(const string &str);
template <typename T> inline optional<unsigned int> indexOf(const T& itm, const vector<T>& vec);
template <typename T, size_t N> inline optional<unsigned int> indexOf(const T& itm, const T (&arr)[N]);
template <typename T, size_t N> inline constexpr size_t arraySize(T (&)[N]);
template <typename T> inline bool containsVal(const T& itm, const vector<T>& vec);

// MARK: CLASSES
//| CLASSES
// FIXME: Fix unfinished button logic!
class ButtonRenderer {
    private:
        Color color = WHITE;
        Rectangle rect = {0, 0, 0, 0};
        Rectangle outlineRect = {0, 0, 0, 0};
        float size;
        float outlineSize;
        bool selected;
        bool enabled;
        ButtonType type;
        Vector2 pos;
    public:
        Texture2D texture;
        float opacity;
        Color selectionColor;
        ButtonRenderer(Vector2 _pos, ButtonType _type, bool _enabled, float _size, bool _selected, Rectangle _rect, Rectangle _outlineRect, Texture2D _texture, float _opacity=255, Color _selectionColor=WHITE) {
            pos = _pos;
            enabled = _enabled;
            size = _size;
            selected = _selected;
            type = _type;
            rect = _rect;
            outlineRect = _outlineRect;
            texture = _texture;
            opacity = _opacity;
            selectionColor = _selectionColor;
        }
        void Render() {
            if (enabled) {
                Color curColor = {color.r, color.g, color.b, opacity};
                if (selected) {
                    Color curSelectionColor = {selectionColor.r, selectionColor.g, selectionColor.b, opacity};
                    if (type == CIRCLE) {
                        DrawCircle(pos.x, pos.y, outlineSize, curSelectionColor);
                    } else {
                        DrawRectangleRec(outlineRect, curSelectionColor);
                    }
                }
                DrawTexture(texture, pos.x - (texture.width / 2), pos.y - (texture.height / 2), curColor);
            }
        }
        void SetOutlineSize(float outline_size) {
            if (type == CIRCLE) {
                DEBUG_LOG("IS CIRCLE");
                outlineSize = size + outline_size;
            } else {
                outlineRect = {(pos.x - (texture.width / 2)) - (outline_size / 2), (pos.y - (texture.height / 2)) - (outline_size / 2), static_cast<float>(texture.width + outline_size), static_cast<float>(texture.height + outline_size)};
            }
        }
};
class ButtonInteraction {
    private:
    public:
        ButtonInteraction() {

        }
};
class Button {
    private:
        Rectangle rect = {0, 0, 0, 0};
        Rectangle outlineRect = {0, 0, 0, 0};
        float size;
        float outlineSize;
    public:
        string name;
        Vector2 pos;
        ButtonType type;
        vector<MenuState> available_states;
        bool enabled = true;
        bool selected = false;
        bool selectable;
        ButtonRenderer renderer;
        ButtonInteraction interactor;
        Button(string btn_name, Vector2 btn_pos, ButtonType btn_type, bool btn_selectable = true, ButtonRenderer btn_renderer, Texture2D btn_texture, vector<MenuState> btn_available_states) {
            name = btn_name;
            pos = btn_pos;
            type = btn_type;
            available_states = btn_available_states;
            selectable = btn_selectable;
            size = btn_texture.width / 2;
            if (type == SQUARE) {
                rect = {pos.x - (btn_texture.width / 2), pos.y - (btn_texture.height / 2), static_cast<float>(btn_texture.width), static_cast<float>(btn_texture.height)};
                outlineRect = {(pos.x - (btn_texture.width / 2)) - 5, (pos.y - (btn_texture.height / 2)) - 5, static_cast<float>(btn_texture.width + 10), static_cast<float>(btn_texture.height + 10)};
            } else {
                outlineSize = size + (size * 0.1);
            }
            if (type == SQUARE) {
                renderer = ButtonRenderer(pos, type, enabled, size, selected, rect, outlineRect, btn_texture);
            }
        }
        void AvailableVisibility(MenuState menu_state) {
            enabled = containsVal(menu_state, available_states);
        }
        inline bool IsHovered(Vector2 mouse_pos) {
            if (type == CIRCLE) {
                return CheckCollisionPointCircle(mouse_pos, pos, size) && enabled;
            } else {
                return CheckCollisionPointRec(mouse_pos, rect) && enabled;
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
            enabled = false;
        }
        void Show() {
            enabled = true;
        }
        void ToggleVisibility() {
            enabled = !enabled;
        }
        void SetSelectionColor(Color slct_color) {
            selection_color = slct_color;
        }
};

// MARK: Variables
//| Global Variables
// For note detection
vector<Button> buttons = {};
// Treble Clef
Note trebleLines[11] = {
    {NoteName::E, Accidental::Natural, 4}, // bottom line
    {NoteName::G, Accidental::Natural, 4},
    {NoteName::B, Accidental::Natural, 4},
    {NoteName::D, Accidental::Natural, 5},
    {NoteName::F, Accidental::Natural, 5},
    {NoteName::A, Accidental::Natural, 5}, // top line
    {NoteName::C, Accidental::Natural, 6}, // ledger above
    {NoteName::E, Accidental::Natural, 6},
    {NoteName::G, Accidental::Natural, 6},
    {NoteName::B, Accidental::Natural, 6},
    {NoteName::D, Accidental::Natural, 7}
};

Note trebleSpaces[10] = {
    {NoteName::F, Accidental::Natural, 4}, // bottom space
    {NoteName::A, Accidental::Natural, 4},
    {NoteName::C, Accidental::Natural, 5},
    {NoteName::E, Accidental::Natural, 5},
    {NoteName::G, Accidental::Natural, 5},
    {NoteName::B, Accidental::Natural, 5},
    {NoteName::D, Accidental::Natural, 6},
    {NoteName::F, Accidental::Natural, 6},
    {NoteName::A, Accidental::Natural, 6},
    {NoteName::C, Accidental::Natural, 7}
};

// Bass Clef
Note baseLines[9] = {
    {NoteName::G, Accidental::Natural, 2}, // bottom line
    {NoteName::B, Accidental::Natural, 2},
    {NoteName::D, Accidental::Natural, 3},
    {NoteName::F, Accidental::Natural, 3},
    {NoteName::A, Accidental::Natural, 3},
    {NoteName::C, Accidental::Natural, 4},
    {NoteName::E, Accidental::Natural, 4},
    {NoteName::G, Accidental::Natural, 4},
    {NoteName::B, Accidental::Natural, 4} // top line
};

Note baseSpaces[10] = {
    {NoteName::A, Accidental::Natural, 2}, // bottom space
    {NoteName::C, Accidental::Natural, 3},
    {NoteName::E, Accidental::Natural, 3},
    {NoteName::G, Accidental::Natural, 3},
    {NoteName::B, Accidental::Natural, 3},
    {NoteName::D, Accidental::Natural, 4},
    {NoteName::F, Accidental::Natural, 4},
    {NoteName::A, Accidental::Natural, 4},
    {NoteName::C, Accidental::Natural, 5},
    {NoteName::E, Accidental::Natural, 5} // top space
};

int sizeTrebleLines = sizeof(trebleLines) / sizeof(trebleLines[0]);
int sizeTrebleSpaces = sizeof(trebleSpaces) / sizeof(trebleSpaces[0]);
int sizeBaseLines = sizeof(baseLines) / sizeof(baseLines[0]);
int sizeBaseSpaces = sizeof(baseSpaces) / sizeof(baseSpaces[0]);
// Timers and frames
int newNoteTimer = {};
int checkNoteTimer = {};
int TIMER = {};
MenuState menuState = {FLASHCARD_MENU};

//| Template variables
Texture2D grandStaffTexture;
Texture2D sharpTexture;
Texture2D flatTexture;
Texture2D naturalTexture;
Texture2D noteTexture;
Texture2D ledgerTexture;
Texture2D settingsTexture;
Texture2D closeSettingsTexture;
Texture2D flatButtonTexture;
Texture2D sharpButtonTexture;
Font roboto;
Font bravuraText;
Font bravura;

//| Colors
Color Transparent = {255, 255, 255, 127};
Color sharpCircleCol = GRAY;
Color flatCircleCol = GRAY;

// MARK: FUNCTIONS:











// MARK: Utilities
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
optional<unsigned int> indexOf(const T& itm, const T (&arr)[N]) {
    auto it = find(arr, arr + N, itm);
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
    uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

// Wait in miliseconds
void wait(int time) {
    this_thread::sleep_for(chrono::milliseconds(time));
}

// Changes timers and wait 1/60th of a second.
void tick() {
    wait(FRAME);
    TIMER += 1;
    TIMER = TIMER == 60 ? 0 : TIMER;
    newNoteTimer -= 1;
    newNoteTimer = newNoteTimer < 0 ? 0 : newNoteTimer;
    checkNoteTimer -= 1;
    checkNoteTimer = checkNoteTimer < 0 ? 0 : checkNoteTimer;
}

// MARK: Assets

Texture2D loadAndResize(const char* path, float scaleFactorX, float scaleFactorY) {
    string filePath = string(ASSET_PATH) + path;
    Image img = LoadImage(filePath.c_str());
    ImageResize(&img, img.width * scaleFactorX, img.height * scaleFactorY);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResize(const char* path, float scaleFactor) {
    string filePath = string(ASSET_PATH) + path;
    Image img = LoadImage(filePath.c_str());
    ImageResize(&img, img.width * scaleFactor, img.height * scaleFactor);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResizeRelative(const char* path, float scaleFactor, float standardImgSize) {
    string filePath = string(ASSET_PATH) + path;
    Image img = LoadImage(filePath.c_str());
    float relativeSizeX = img.width * (standardImgSize / img.width);
    float relativeSizeY = img.height * (standardImgSize / img.width);
    ImageResize(&img, relativeSizeX * scaleFactor, relativeSizeY * scaleFactor);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResizeRelative(const char* path, float scaleFactorX, float scaleFactorY, float standardImgSize) {
    string filePath = string(ASSET_PATH) + path;
    Image img = LoadImage(filePath.c_str());
    float relativeSizeX = img.width * (standardImgSize / img.width);
    float relativeSizeY = img.height * (standardImgSize / img.width);
    ImageResize(&img, relativeSizeX * scaleFactorX, relativeSizeY * scaleFactorY);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// Loads assets
void loadAssets() {
    string font_path = string(ASSET_PATH) + "Roboto-Black.ttf";
    roboto = LoadFontEx(font_path.c_str(), 128, NULL, 0);
    font_path = string(ASSET_PATH) + "BravuraText.otf";
    bravura = LoadFontEx(font_path.c_str(), 128, NULL, 0);
    float standardSize = 100;


    grandStaffTexture = loadAndResizeRelative("GrandStaff.png", 18, standardSize);
    sharpTexture = loadAndResizeRelative("sharp.png", 0.25f, standardSize);
    flatTexture = loadAndResizeRelative("Flat.png", 0.25f, standardSize);
    naturalTexture = loadAndResizeRelative("natural.png", 0.25f, standardSize);
    noteTexture = loadAndResizeRelative("QuarterNote.png", 0.5f, standardSize);
    ledgerTexture = loadAndResizeRelative("LedgerLine.png", 0.8333f, 0.6666f, standardSize);
    settingsTexture = loadAndResizeRelative("SettingsIcon.png", 0.6666f, standardSize);
    closeSettingsTexture = loadAndResizeRelative("CloseButton.png", 0.6666f, standardSize);
    flatButtonTexture = loadAndResizeRelative("Flat.png", 0.3f, standardSize);
    sharpButtonTexture = loadAndResizeRelative("sharp.png", 0.3f, standardSize);
}


// MARK: Rendering

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

void drawAccidental(Accidental accidental, int x, int y, Color tint) {
    switch (accidental) {
        case Accidental::Sharp: DrawTexture(sharpTexture, x - 30, y - ((sharpTexture.height / 2) - 5), tint); break;
        case Accidental::Flat: DrawTexture(flatTexture,  x - 30, y - ((flatTexture.height / 2) + 13), tint); break;
        case Accidental::Natural: DrawTexture(naturalTexture, x - 30, y - ((naturalTexture.height / 2) - 5), tint); break;
    }
}

// Displays whichever note is passed into it.
// The tint of the textures is changeable, but defaults to White.
void drawNote(Note note, Color tint = WHITE) {
    // 44.5 pixels between each line
    // 495 is the bottom line for Treble Clef, or E4
    // 803 is the bottom line for Base Clef, or G2
    int y = 0;
    int x = 500;
    float noteStep = 44.5;
    int accidentalXOffset = -30;
    int sharpYOffset = -5;
    int flatYOffset = 13;
    int ledgerXOffset = -15;
    int ledgerYOffset = 4;
    int noteYOffset = 45;

    //# Note Decoding
    Accidental accidental = note.accidental;
    Note testNote = {note.name, Accidental::Natural, note.octave};
    bool isTrebleLine = indexOf(testNote, trebleLines) != nullopt;
    bool isTrebleSpace = indexOf(testNote, trebleSpaces) != nullopt;
    bool isTreble = isTrebleLine || isTrebleSpace;
    bool isBaseLine = indexOf(testNote, baseLines) != nullopt;
    bool isBaseSpace = indexOf(testNote, baseSpaces) != nullopt;
    bool isBase = isBaseLine || isBaseSpace;

    //# Note Rendering
    if (isTreble) {
        // FIXME: The positioning is off
        y = isTrebleLine ? 492 - (noteStep * indexOf(testNote, trebleLines).value_or(-1)) : 468 - (noteStep * indexOf(testNote, trebleSpaces).value_or(-1));
        if (testNote.name == NoteName::C && testNote.octave == 4) {
            accidentalXOffset -= 10;
            DrawTexture(ledgerTexture, x + ledgerXOffset, y + ledgerYOffset, tint);
        }
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != Accidental::Natural) {
            drawAccidental(accidental, x, y, tint);
        }
    } else if (isBase) {  //# Base Clef
        // FIXME: The positioning is off
        y = isBaseLine ? 800 - (noteStep * indexOf(testNote, baseLines).value_or(-1)) :  822 - (noteStep * indexOf(testNote, baseSpaces).value_or(-1));
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != Accidental::Natural) {
            drawAccidental(accidental, x, y, tint);
        }
    }
}

// MARK: Note Logic

Note convertToLegalNote(Note inputNote) {
    Note testNote = {inputNote.name, inputNote.accidental, 1};
    if (testNote == Note{NoteName::F, Accidental::Flat, 1}) {
        DEBUG_LOG("Fb");
        return Note{NoteName::E, Accidental::Natural, inputNote.octave};
    }
    else if (testNote == Note{NoteName::C, Accidental::Flat, 1}) {
        DEBUG_LOG("Cb");
        return Note{NoteName::B, Accidental::Natural, inputNote.octave};
    }
    else if (testNote == Note{NoteName::E, Accidental::Sharp, 1}) {
        DEBUG_LOG("E#");
        return Note{NoteName::F, Accidental::Natural, inputNote.octave};
    }
    else if (testNote == Note{NoteName::B, Accidental::Sharp, 1}) {
        DEBUG_LOG("B#");
        return Note{NoteName::C, Accidental::Natural, inputNote.octave};
    }
    else {
        return inputNote;
    }
}

void setNewNote(Note *_correctNote, float accidentalProbability = 0.5f) {
    if (randint(0, 1)) {
        if (randint(0, 1)) {
            *_correctNote = trebleLines[(randint(0, sizeTrebleLines - 1))];
        } else {
            *_correctNote = trebleSpaces[(randint(0, sizeTrebleSpaces - 1))];
        }
    } else {
        if (randint(0, 1)) {
            *_correctNote = baseSpaces[(randint(0, sizeBaseSpaces - 1))];
        } else {
            *_correctNote = baseLines[(randint(0, sizeBaseLines - 1))];
        }
    }
    if (randint(0, ceil(1 / accidentalProbability)) == 0) {
        if (SHARPS) {
            *_correctNote = {(*_correctNote).name, Accidental::Sharp, (*_correctNote).octave};
        } else {
            *_correctNote = {(*_correctNote).name, Accidental::Flat, (*_correctNote).octave};
        }
        *_correctNote = convertToLegalNote(*_correctNote);
    }
}

// Enharmonic table for one octave using indexing
Note enharmonicSwap(Note note) {
    Accidental accidental = note.accidental;
    if (SHARPS && accidental == Accidental::Flat) {
        if (note.name == NoteName::D) return Note{NoteName::C, Accidental::Sharp, note.octave};
        if (note.name == NoteName::E) return Note{NoteName::D, Accidental::Sharp, note.octave};
        if (note.name == NoteName::G) return Note{NoteName::F, Accidental::Sharp, note.octave};
        if (note.name == NoteName::A) return Note{NoteName::G, Accidental::Sharp, note.octave};
        if (note.name == NoteName::B) return Note{NoteName::A, Accidental::Sharp, note.octave};
    } else if (accidental == Accidental::Sharp && !SHARPS) {
        if (note.name == NoteName::C) return Note{NoteName::D, Accidental::Flat, note.octave};
        if (note.name == NoteName::D) return Note{NoteName::E, Accidental::Flat, note.octave};
        if (note.name == NoteName::F) return Note{NoteName::G, Accidental::Flat, note.octave};
        if (note.name == NoteName::G) return Note{NoteName::A, Accidental::Flat, note.octave};
        if (note.name == NoteName::A) return Note{NoteName::B, Accidental::Flat, note.octave};
    }
    return note;
}

// Checks whether the correct note is being played.
// Handles new target note logic and renders the target note on the screen.
void checkNote(Note *correctNote) {
    Accidental accidental = correctNote->accidental;
    Note testNote = {(*correctNote).name, accidental, 1};
    char octave = correctNote->octave;
    *correctNote = enharmonicSwap(*correctNote);
    if (CURRENTNOTE == *correctNote && checkNoteTimer == 0) {
        checkNoteTimer = 30;
        newNoteTimer = 10;
    }
    if (newNoteTimer == 1) {
        DEBUG_LOG("HIT NOTE");
        setNewNote(&*correctNote);
        DEBUG_LOG("NEW NOTE: " << *correctNote);
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
        DEBUG_LOG(mouse.x << ", " << mouse.y);
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
        } else if (hovered_btn == "do_flats") {
            SHARPS = false;
        }
        for (Button &button : buttons) {
            if (hovered_btn == button.name) {
                DEBUG_LOG(button.name << " WAS SELECTED");
                button.selected = true;
            } else {
                button.selected = false;
            }
        }
    }
}

void selectAccidental() {
    for (Button &button : buttons) {
        if (button.name == "do_sharps") {
            if (SHARPS) {
                button.selected = true;
            } else {
                button.selected = false;
            }
        }
        if (button.name == "do_flats") {
            if (SHARPS) {
                button.selected = false;
            } else {
                button.selected = true;
            }
        }
    }
}

void flashcardMenu(Note &crct_note) {
    drawStaff(grandStaffTexture);
    checkNote(&crct_note);
    drawNote(CURRENTNOTE, Transparent);
    drawNote(crct_note, Color {255, 255, 255, 75});
}

void settingsMenu() {
    selectAccidental();
}

void testAllNotes() {
    for (Note note : allNotes) {
        ClearBackground(RAYWHITE);
        drawStaff(grandStaffTexture);
        drawNote(note, WHITE);
        // DEBUG_LOG(to_string(Note));
        wait(1000);
    }
}

// MARK: MAIN

// Handles all GUI logic
void RunGUI() {
    InitWindow(WINWIDTH, WINHEIGHT, "Piano Flashcard App");
    SetTargetFPS(60);
    SetTextLineSpacing(16);
    loadAssets();
    Button settingsButton("settings", Vector2({WINWIDTH - 80, 80}), CIRCLE, settingsTexture, vector<MenuState> {FLASHCARD_MENU}, false);
    buttons.push_back(settingsButton);
    Button closeSettingsButton("close_settings", Vector2({WINWIDTH - 80, 80}), CIRCLE, closeSettingsTexture, vector<MenuState> {SETTINGS_MENU}, false);
    buttons.push_back(closeSettingsButton);
    Button sharpButton("do_sharps", Vector2({(WINWIDTH / 2) - 50, WINHEIGHT / 2}), SQUARE, sharpButtonTexture, vector<MenuState> {SETTINGS_MENU});
    sharpButton.SetOutlineSize(20);
    buttons.push_back(sharpButton);
    Button flatButton("do_flats", Vector2({(WINWIDTH / 2) + 50, WINHEIGHT / 2}), SQUARE, flatButtonTexture, vector<MenuState> {SETTINGS_MENU});
    flatButton.SetOutlineSize(20);
    buttons.push_back(flatButton);

    string hoveredBtn = "???";
    newNoteTimer = 0;
    Note correctNote = {NoteName::F, Accidental::Sharp, 4};
    bool was_clicked = false;

    // testAllNotes();

    while (!WindowShouldClose()) {
        buttonLogic(mouseLogic(&hoveredBtn), hoveredBtn);
        ClearBackground(RAYWHITE);
        drawButtons();
        DrawTextEx(bravuraText, TREBLE_CLEF, Vector2({WINWIDTH / 2, WINHEIGHT / 2}), 30, 2, WHITE);

        if (menuState == FLASHCARD_MENU) {
            flashcardMenu(correctNote);
        } else if (menuState == SETTINGS_MENU) {
            settingsMenu();
        }
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
