#pragma once

#include <Arduino.h>

struct Track {
  const char* title;
  const char* author;
  uint16_t fileIndex;  // must match the mp3 numbering on the SD card (0001.mp3, 0002.mp3, ...)
};

// C418 - Minecraft Volume Alpha & Beta, in SD-card file order (0001.mp3..0054.mp3).
static const Track TRACKS[] = {
    {"Beginning", "C418", 1},
    {"Cat", "C418", 2},
    {"Chris", "C418", 3},
    {"Clark", "C418", 4},
    {"Danny", "C418", 5},
    {"Death", "C418", 6},
    {"Dog", "C418", 7},
    {"Door", "C418", 8},
    {"Droopy likes Ricochet", "C418", 9},
    {"Droopy likes your Face", "C418", 10},
    {"Dry Hands", "C418", 11},
    {"Excuse", "C418", 12},
    {"Haggstrom", "C418", 13},
    {"Key", "C418", 14},
    {"Living Mice", "C418", 15},
    {"Mice on Venus", "C418", 16},
    {"Minecraft", "C418", 17},
    {"Moog City", "C418", 18},
    {"Oxygene", "C418", 19},
    {"Subwoofer Lullaby", "C418", 20},
    {"Sweden", "C418", 21},
    {"Thirteen", "C418", 22},
    {"Wet Hands", "C418", 23},
    {"Equinoxe", "C418", 24},
    {"Alpha", "C418", 25},
    {"Aria Math", "C418", 26},
    {"Ballad of the Cats", "C418", 27},
    {"Beginning 2", "C418", 28},
    {"Biome Fest", "C418", 29},
    {"Blind Spots", "C418", 30},
    {"Blocks", "C418", 31},
    {"Chirp", "C418", 32},
    {"Concrete Halls", "C418", 33},
    {"Dead Voxel", "C418", 34},
    {"Dreiton", "C418", 35},
    {"Eleven", "C418", 36},
    {"Far", "C418", 37},
    {"Flake", "C418", 38},
    {"Floating Trees", "C418", 39},
    {"Haunt Muskie", "C418", 40},
    {"Intro", "C418", 41},
    {"Ki", "C418", 42},
    {"Kyoto", "C418", 43},
    {"Mall", "C418", 44},
    {"Mellohi", "C418", 45},
    {"Moog City 2", "C418", 46},
    {"Mutation", "C418", 47},
    {"Stal", "C418", 48},
    {"Strad", "C418", 49},
    {"Taswell", "C418", 50},
    {"The End", "C418", 51},
    {"Wait", "C418", 52},
    {"Ward", "C418", 53},
    {"Warmth", "C418", 54},
};
constexpr size_t TRACK_COUNT = sizeof(TRACKS) / sizeof(TRACKS[0]);
