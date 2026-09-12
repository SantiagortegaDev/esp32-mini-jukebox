#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#include "tracks.h"

class Display {
 public:
  static constexpr uint8_t NUM_BOOT_FRAMES = 20;

  bool begin();

  void showBoot(uint8_t frame);
  void showPlaying(const Track& track, bool paused, uint8_t frame);
  void showList(const Track tracks[], size_t count, size_t cursorIndex);
  void showError(const char* message);

 private:
  void drawDisc(int16_t cx, int16_t cy, uint8_t frame);

  Adafruit_SSD1306 _oled{128, 64, &Wire, -1};
};
