#pragma once
#include <string>
#include <atomic>
#include "notes.hpp"
#include <vector>

extern std::vector<Note> allNotes;
extern Note CURRENTNOTE;
extern std::atomic<bool> RUNNINGPROGRAM;
extern std::atomic<bool> SHARPS;
extern std::atomic<double> VOLUMETHRESHOLD;