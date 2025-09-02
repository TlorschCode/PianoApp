#ifndef ACCIDENTAL_HPP
#define ACCIDENTAL_HPP

#include <string>
#include <ostream>

// Strongly typed scoped enums
enum class Accidental { Natural, Sharp, Flat };
enum class NoteName { C, D, E, F, G, A, B };

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

// Convert enums to strings
inline std::string to_string(Accidental acc) {
    switch (acc) {
        case Accidental::Natural: return "N";
        case Accidental::Sharp:   return "#";
        case Accidental::Flat:    return "b";
    }
    return "?";
}

inline std::string to_string(NoteName name) {
    switch (name) {
        case NoteName::C: return "C";
        case NoteName::D: return "D";
        case NoteName::E: return "E";
        case NoteName::F: return "F";
        case NoteName::G: return "G";
        case NoteName::A: return "A";
        case NoteName::B: return "B";
    }
    return "?";
}

// Convert full Note to string (optional helper)
inline std::string to_string(const Note& note) {
    return to_string(note.name) + 
           (note.accidental == Accidental::Natural ? "" : to_string(note.accidental)) + 
           std::to_string(note.octave);
}

// Stream output operators (optional, for std::cout << ...)
inline std::ostream& operator<<(std::ostream& os, Accidental acc) {
    return os << to_string(acc);
}

inline std::ostream& operator<<(std::ostream& os, NoteName name) {
    return os << to_string(name);
}

inline std::ostream& operator<<(std::ostream& os, const Note& note) {
    return os << to_string(note);
}

#endif // ACCIDENTAL_HPP