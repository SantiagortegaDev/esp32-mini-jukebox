#include "player.h"

bool Player::begin(HardwareSerial& serial) {
  if (!_df.begin(serial, /*isACK=*/true, /*doReset=*/true)) {
    return false;
  }
  _df.volume(_volume);
  return true;
}

void Player::play(uint16_t fileIndex) { _df.play(fileIndex); }

void Player::pause() { _df.pause(); }

void Player::resume() { _df.start(); }

void Player::volumeUp() {
  if (_volume < MAX_VOLUME) {
    _volume++;
    _df.volume(_volume);
  }
}

void Player::volumeDown() {
  if (_volume > MIN_VOLUME) {
    _volume--;
    _df.volume(_volume);
  }
}

bool Player::justFinished() {
  return _df.available() && _df.readType() == DFPlayerPlayFinished;
}
