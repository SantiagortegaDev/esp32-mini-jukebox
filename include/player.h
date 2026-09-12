#pragma once

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

class Player {
 public:
  bool begin(HardwareSerial& serial);

  void play(uint16_t fileIndex);
  void pause();
  void resume();
  void volumeUp();
  void volumeDown();
  bool justFinished();

 private:
  static constexpr uint8_t MIN_VOLUME = 0;
  static constexpr uint8_t MAX_VOLUME = 30;

  DFRobotDFPlayerMini _df;
  uint8_t _volume = 20;
};
