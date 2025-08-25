#ifndef ACCIDENTAL_HPP
#define ACCIDENTAL_HPP

// Strongly typed scoped enum
enum class Accidental {Natural, Sharp, Flat};
enum class NoteName {C, D, E, F, G, A, B};

struct Note {
    NoteName name;
    Accidental accidental;
    int octave;
    bool operator==(const Note& other) const {
        return name == other.name &&
               accidental == other.accidental &&
               octave == other.octave;
    }
};
#endif // ACCIDENTAL_HPP