#pragma once

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// An animation is either procedural (drawn each frame with GFX calls) or
// bitmap-based (steps through a PROGMEM array of full-screen 128x64 frames).
// Only one of drawProcedural/bitmapFrames is set per entry.
struct Animation {
  uint16_t frameDelayMs;
  uint8_t frameCount;
  void (*drawProcedural)(Adafruit_SSD1306&, uint8_t frame);  // nullptr for bitmap animations
  const uint8_t* const* bitmapFrames;                        // nullptr for procedural animations
};

extern const Animation ANIMATIONS[];
extern const uint8_t ANIMATION_COUNT;

enum class AnimationMode : uint8_t { RANDOM, FIXED, NONE };

// Config: how the active animation is picked. NONE means the PLAYING screen
// shows only the centered title/author instead of an animation.
constexpr AnimationMode ANIMATION_MODE = AnimationMode::RANDOM;
constexpr uint8_t FIXED_ANIMATION_INDEX = 0;  // used only when ANIMATION_MODE == FIXED

namespace Animations {

void begin();          // seeds the active animation per ANIMATION_MODE, call once in setup()
void onTrackStart();    // re-rolls a new random animation if ANIMATION_MODE == RANDOM

// Runtime override, callable from anywhere in the firmware (buttons, future
// features, etc.) to change what's showing regardless of ANIMATION_MODE.
// -1 means "no animation" (falls back to centered title/author).
void setActive(int8_t index);
int8_t active();

void tick(unsigned long nowMs);
bool isBitmap();
const uint8_t* bitmapFrame();
void drawProcedural(Adafruit_SSD1306& oled);

}  // namespace Animations
