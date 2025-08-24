#ifdef DEBUG
    #include <iostream>
    #define DEBUG_LOG(x) std::cout << "  DEBUG:  " << x << "\n"
#else
    #define DEBUG_LOG(x)
#endif

// TODO: Add volume slider

//| INIT
#include <string>
#include <atomic>
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
inline string to_lowercase(const string &str);
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
        Rectangle outlineRect = {0, 0, 0, 0};
        Color selection_color = {175, 175, 175, 255};
        float size;
        float outlineSize;
    public:
        string name;
        Vector2 pos;
        ButtonType type;
        Texture2D texture;
        vector<MenuState> available_states;
        bool shown = true;
        bool selected;
        bool selectable;
        Button(string btn_name, Vector2 btn_pos, ButtonType btn_type, Texture2D btn_texture, vector<MenuState> btn_available_states, bool btn_selectable = true) {
            name = btn_name;
            pos = btn_pos;
            type = btn_type;
            texture = btn_texture;
            available_states = btn_available_states;
            selected = false;
            selectable = btn_selectable;
            size = texture.width / 2;
            if (type == SQUARE) {
                rect = {pos.x - (texture.width / 2), pos.y - (texture.height / 2), static_cast<float>(texture.width), static_cast<float>(texture.height)};
                outlineRect = {(pos.x - (texture.width / 2)) - 5, (pos.y - (texture.height / 2)) - 5, static_cast<float>(texture.width + 10), static_cast<float>(texture.height + 10)};
            } else {
                outlineSize = size + (size * 0.1);
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
        void SetOcapacity(float target_amnt) {
            ocapacity = target_amnt;
            color = {255, 255, 255, static_cast<unsigned char>(ocapacity)};
        }
        void AvailableVisibility(MenuState menu_state) {
            shown = containsVal(menu_state, available_states);
        }
        void Render() {
            if (shown) {
                if (selected && selectable) {
                    if (type == CIRCLE) {
                        DrawCircle(pos.x, pos.y, outlineSize, selection_color);
                    } else {
                        DrawRectangleRec(outlineRect, selection_color);
                    }
                }
                DrawTexture(texture, pos.x - (texture.width / 2), pos.y - (texture.height / 2), color);
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
        // Sets where the circle 
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
Texture2D flatButtonTexture;
Texture2D sharpButtonTexture;
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

Texture2D loadAndResize(const std::string &path, float scaleFactorX, float scaleFactorY) {
    Image img = LoadImage(path.c_str());
    ImageResize(&img, img.width * scaleFactorX, img.height * scaleFactorY);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResize(const std::string &path, float scaleFactor) {
    Image img = LoadImage(path.c_str());
    ImageResize(&img, img.width * scaleFactor, img.height * scaleFactor);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResizeRelative(const std::string &path, float scaleFactor, float standardImgSize) {
    Image img = LoadImage(path.c_str());
    float relativeSizeX = img.width * (standardImgSize / img.width);
    float relativeSizeY = img.height * (standardImgSize / img.width);
    ImageResize(&img, relativeSizeX * scaleFactor, relativeSizeY * scaleFactor);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D loadAndResizeRelative(const std::string &path, float scaleFactorX, float scaleFactorY, float standardImgSize) {
    Image img = LoadImage(path.c_str());
    float relativeSizeX = img.width * (standardImgSize / img.width);
    float relativeSizeY = img.height * (standardImgSize / img.width);
    ImageResize(&img, relativeSizeX * scaleFactorX, relativeSizeY * scaleFactorY);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// Loads assets
void loadAssets() {
    roboto = LoadFontEx("assets/Roboto-Black.ttf", 128, NULL, 0);
    float standardSize = 100;

    grandStaffTexture = loadAndResizeRelative("assets/GrandStaff.png", 18, standardSize);
    sharpTexture = loadAndResizeRelative("assets/sharp.png", 0.25f, standardSize);
    flatTexture = loadAndResizeRelative("assets/Flat.png", 0.25f, standardSize);
    naturalTexture = loadAndResizeRelative("assets/natural.png", 0.25f, standardSize);
    noteTexture = loadAndResizeRelative("assets/QuarterNote.png", 0.5f, standardSize);
    ledgerTexture = loadAndResizeRelative("assets/LedgerLine.png", 0.8333f, 0.6666f, standardSize);
    settingsTexture = loadAndResizeRelative("assets/SettingsIcon.png", 0.6666f, standardSize);
    closeSettingsTexture = loadAndResizeRelative("assets/CloseButton.png", 0.6666f, standardSize);
    flatButtonTexture = loadAndResizeRelative("assets/Flat.png", 0.3f, standardSize);
    sharpButtonTexture = loadAndResizeRelative("assets/sharp.png", 0.3f, standardSize);
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

inline char getAccidental(string note) {
    return (note.length() == 3) ? note.at(1) : '\0';
}

inline string getNoteAndOctave(string note) {
    return (note.length() == 3) ? (note.substr(0, 1) + note.back()) : note;
}

inline string getNote(string note) {
    return (note).substr(0, 1);
}

inline char getOctave(string note) {
    return (note).back();
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
    int sharpYOffset = -5;
    int flatYOffset = 13;
    int ledgerXOffset = -15;
    int ledgerYOffset = 4;
    int noteYOffset = 45;

    //# Note Decoding
    string note = getNoteAndOctave(useNote);
    char accidental = getAccidental(useNote);
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
            accidentalXOffset -= 10;
            DrawTexture(ledgerTexture, x + ledgerXOffset, y + ledgerYOffset, tint);
        }
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + accidentalXOffset, y - ((sharpTexture.height / 2) + sharpYOffset), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + accidentalXOffset, y - ((flatTexture.height / 2) + flatYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + accidentalXOffset,  y - ((naturalTexture.height / 2) + sharpYOffset), tint);
                    break;
            }
        }
    } else if (isBase) {  //# Base Clef
        y = isBaseLine ? 800 - (noteStep * indexOf(note, baseLines).value_or(-1)) :  822 - (noteStep * indexOf(note, baseSpaces).value_or(-1));
        DrawTexture(noteTexture, x, y - ((noteTexture.height / 2) + noteYOffset), tint);
        if (accidental != '\0') {
            switch (accidental) {
                case '#':
                    DrawTexture(sharpTexture, x + accidentalXOffset, y - ((sharpTexture.height / 2) + sharpYOffset), tint);
                    break;
                case 'b':
                    DrawTexture(flatTexture, x + accidentalXOffset, y - ((flatTexture.height / 2) + flatYOffset), tint);
                    break;
                case 'N':
                    DrawTexture(naturalTexture, x + accidentalXOffset,  y - ((naturalTexture.height / 2) + sharpYOffset), tint);
                    break;
            }
        }
    }
}

// Checks whether the correct note is being played.
// Handles new target note logic and renders the target note on the screen.
void checkNote(string *correctNote) {
    char octave = getOctave(*correctNote);
    char accidental = getAccidental(*correctNote);
    string note = getNote(*correctNote);
    if (SHARPS) {
        if (accidental == 'b') {
            DEBUG_LOG(note + accidental);
            int index = indexOf((note + accidental), flatNotes).value_or(-1);
            DEBUG_LOG(index);
            DEBUG_LOG(sharpNotes[index]);
            *correctNote = sharpNotes[index] + octave;
        }
    } else {
        if (accidental == '#') {
            DEBUG_LOG(note + accidental);
            int index = indexOf((note + accidental), sharpNotes).value_or(-1);
            DEBUG_LOG(index);
            DEBUG_LOG(flatNotes[index]);
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

string convertToLegalNote(string inputNote) { // FIXME: Note isn't displaying
    if ((getNote(inputNote) + getAccidental(inputNote)) == "Fb") {
        return "E" + getOctave(inputNote);
        DEBUG_LOG("Fb");
    }
    else if ((getNote(inputNote) + getAccidental(inputNote)) == "Cb") {
        return "B" + getOctave(inputNote);
        DEBUG_LOG("Cb");
    }
    else if ((getNote(inputNote) + getAccidental(inputNote)) == "E#") {
        return "F" + getOctave(inputNote);
        DEBUG_LOG("E#");
    }
    else if ((getNote(inputNote) + getAccidental(inputNote)) == "B#") {
        return "C" + getOctave(inputNote);
        DEBUG_LOG("B#");
    }
    else {
        return inputNote;
    }
}

void flashcardMenu(string &crct_note) {
    drawButtons();
    // crct_note = convertToLegalNote(crct_note);
    checkNote(&crct_note);
    drawStaff(grandStaffTexture);
    drawNote(CURRENTNOTE, Transparent);
    drawNote(crct_note, Color {255, 255, 255, 75});
}

void settingsMenu() {
    selectAccidental();
}

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
    string correctNote = "G#3";
    bool was_clicked = false;

    while (!WindowShouldClose()) {
        buttonLogic(mouseLogic(&hoveredBtn), hoveredBtn);
        ClearBackground(RAYWHITE);
        drawButtons();

        if (menuState == FLASHCARD_MENU) {
            flashcardMenu(correctNote);
        } else if (menuState == SETTINGS_MENU) {
            settingsMenu();
            // // FIXME: Doesn't work correctly
            // sharpButton.selected = SHARPS;
            // flatButton.selected = !SHARPS;
        }
        // DrawTextEx(roboto, CURRENTNOTE.c_str(), {100, 100}, 40, 2, DARKGRAY);
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
