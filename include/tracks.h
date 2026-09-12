#pragma once

#include <Arduino.h>

struct Track {
  const char* title;
  const char* author;
  uint16_t fileIndex;  // must match the mp3 numbering on the SD card (0001.mp3, 0002.mp3, ...)
};

// Placeholder tracklist - replace titles/authors and make sure fileIndex
// matches the actual mp3 files on the SD card.
static const Track TRACKS[] = {
    {"Track 1", "Unknown Artist", 1},
    {"Track 2", "Unknown Artist", 2},
    {"Track 3", "Unknown Artist", 3},
    {"Track 4", "Unknown Artist", 4},
    {"Track 5", "Unknown Artist", 5},
    {"Track 6", "Unknown Artist", 6},
    {"Track 7", "Unknown Artist", 7},
    {"Track 8", "Unknown Artist", 8},
    {"Track 9", "Unknown Artist", 9},
    {"Track 10", "Unknown Artist", 10},
};
constexpr size_t TRACK_COUNT = sizeof(TRACKS) / sizeof(TRACKS[0]);

// true = shuffle with back-history (PREV always replays the actual previous track)
// false = strict sequential circular order
constexpr bool RANDOM_MODE = true;
