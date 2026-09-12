#pragma once

#include <Arduino.h>

// Debounced button with edge-triggered short/long press detection.
// shortPress() and longPressEdge() are true for exactly one update() call.
class Button {
 public:
  explicit Button(uint8_t pin, unsigned long longPressMs = 600, unsigned long debounceMs = 25);

  void begin();
  void update();

  bool shortPress() const { return _shortEventPending; }
  bool longPressEdge() const { return _longEventPending; }
  bool isHeld() const { return _isPressed; }

 private:
  uint8_t _pin;
  unsigned long _longPressMs;
  unsigned long _debounceMs;

  bool _rawReading = false;
  bool _isPressed = false;
  unsigned long _lastEdgeMs = 0;
  unsigned long _pressStartMs = 0;
  bool _longAlreadyFired = false;

  bool _shortEventPending = false;
  bool _longEventPending = false;
};
