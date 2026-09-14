#pragma once

#include <Arduino.h>

// Single place for every tunable constant in the firmware. Nothing in here
// should need a matching change anywhere else to take effect.

// ---- Hardware pins ----
constexpr uint8_t PIN_PREV = 32;
constexpr uint8_t PIN_SELECT = 25;
constexpr uint8_t PIN_NEXT = 33;

// ---- UI timing ----
constexpr unsigned long REDRAW_INTERVAL_MS = 30;
constexpr unsigned long VOLUME_REPEAT_MS = 150;
constexpr unsigned long BOOT_FRAME_INTERVAL_MS = 100;
// How long a menu (song list or animation picker) can sit idle before it
// auto-closes back to the PLAYING screen.
constexpr unsigned long MENU_IDLE_TIMEOUT_MS = 6000;
constexpr size_t HISTORY_SIZE = 20;

// ---- Track playback ----
// true = shuffle with back-history (PREV always replays the actual previous
// track); false = strict sequential circular order.
constexpr bool RANDOM_MODE = true;

// ---- Animation selection ----
enum class AnimationMode : uint8_t { RANDOM, FIXED, NONE };
// Config: how the active animation is picked at boot / on track change.
// NONE means the PLAYING screen shows only the centered title/author instead
// of an animation. FIXED keeps whatever was picked at boot or chosen in the
// animation menu instead of re-rolling on every track change (RANDOM would
// fight manual selection made through the ANIM_VIEW menu).
constexpr AnimationMode ANIMATION_MODE = AnimationMode::FIXED;
constexpr uint8_t FIXED_ANIMATION_INDEX = 0;  // used only when ANIMATION_MODE == FIXED

// ---- Preview mode ----
// Scripted demo intro: boots straight into a fixed track + animation, forces
// the second track, then behaves like normal RANDOM_MODE (every subsequent
// random pick still gets a random, non-repeating animation, with the pairing
// kept in a shared history so PREV replays the exact track+animation combo).
constexpr bool PREVIEW_MODE = false;
constexpr uint8_t PREVIEW_START_ANIMATION_INDEX = 10;  // animation #11 (1-indexed), the last one
constexpr const char* PREVIEW_START_TRACK_TITLE = "Chirp";
constexpr const char* PREVIEW_SECOND_TRACK_TITLE = "Moog City 2";
