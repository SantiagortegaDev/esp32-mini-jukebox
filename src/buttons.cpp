#include "buttons.h"

Button::Button(uint8_t pin, unsigned long longPressMs, unsigned long debounceMs)
    : _pin(pin), _longPressMs(longPressMs), _debounceMs(debounceMs) {}

void Button::begin() {
  pinMode(_pin, INPUT_PULLUP);
  _rawReading = digitalRead(_pin) == LOW;
  _isPressed = _rawReading;
}

void Button::update() {
  _shortEventPending = false;
  _longEventPending = false;

  bool reading = digitalRead(_pin) == LOW;  // active low
  unsigned long now = millis();

  if (reading != _rawReading) {
    _rawReading = reading;
    _lastEdgeMs = now;
  }

  if ((now - _lastEdgeMs) >= _debounceMs && reading != _isPressed) {
    _isPressed = reading;
    if (_isPressed) {
      _pressStartMs = now;
      _longAlreadyFired = false;
    } else if (!_longAlreadyFired) {
      _shortEventPending = true;
    }
  }

  if (_isPressed && !_longAlreadyFired && (now - _pressStartMs) >= _longPressMs) {
    _longAlreadyFired = true;
    _longEventPending = true;
  }
}
