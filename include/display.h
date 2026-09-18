#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#include "tracks.h"

class Display {
 public:
  // Kept in sync with BOOT_FRAME_COUNT in boot_bitmap.h (the loading-bar
  // bitmap has 11 frames: index 0 is empty, the last one is full).
  static constexpr uint8_t NUM_BOOT_FRAMES = 11;

  bool begin();

  void showBoot(uint8_t frame);
  void showPlaying(const Track& track, bool paused);
  void showAnimSelect(uint8_t index, uint8_t count);
  void showList(const Track tracks[], size_t count, size_t cursorIndex);
  void showError();

 private:
  void drawCentered(const char* text, int16_t y);
  void drawActiveAnimation();

  Adafruit_SSD1306 _oled{128, 64, &Wire, -1};
};
