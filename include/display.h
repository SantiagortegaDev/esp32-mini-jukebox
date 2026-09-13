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
  void showPlaying(const Track& track, bool paused);
  void showList(const Track tracks[], size_t count, size_t cursorIndex);
  void showError(const char* message);

 private:
  void drawCentered(const char* text, int16_t y);

  Adafruit_SSD1306 _oled{128, 64, &Wire, -1};
};
