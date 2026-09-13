#include <Arduino.h>
#include <Wire.h>

#include "animations.h"
#include "buttons.h"
#include "display.h"
#include "player.h"
#include "tracks.h"

namespace {

constexpr uint8_t PIN_PREV = 32;
constexpr uint8_t PIN_SELECT = 25;
constexpr uint8_t PIN_NEXT = 33;

constexpr unsigned long REDRAW_INTERVAL_MS = 30;
constexpr unsigned long VOLUME_REPEAT_MS = 150;
constexpr unsigned long BOOT_FRAME_INTERVAL_MS = 100;
constexpr size_t HISTORY_SIZE = 20;

// Debug switch: while true, NEXT/PREV short-press cycles through
// ANIMATIONS[] instead of skipping tracks. Flip back to false once the
// animation set is finalized to restore normal track-skip behavior.
constexpr bool ANIMATION_DEBUG_MODE = true;

enum class State { BOOT, PLAYING, LIST_VIEW };

Button btnPrev(PIN_PREV);
Button btnSelect(PIN_SELECT);
Button btnNext(PIN_NEXT);

Display display;
Player player;
bool playerReady = false;

State state = State::BOOT;
bool paused = false;
size_t currentTrack = 0;
size_t listCursor = 0;

int history[HISTORY_SIZE];
size_t historyLen = 0;
size_t historyPos = 0;

uint8_t bootFrame = 0;
unsigned long lastRedrawMs = 0;
unsigned long lastBootFrameMs = 0;
unsigned long lastNextRepeatMs = 0;
unsigned long lastPrevRepeatMs = 0;

void historyPush(size_t trackIndex) {
  if (historyLen < HISTORY_SIZE) {
    history[historyLen] = trackIndex;
    historyLen++;
    historyPos = historyLen - 1;
    return;
  }
  for (size_t i = 1; i < HISTORY_SIZE; i++) {
    history[i - 1] = history[i];
  }
  history[HISTORY_SIZE - 1] = trackIndex;
  historyPos = HISTORY_SIZE - 1;
}

// Passing TRACK_COUNT as `excluding` (an always-out-of-range index) means
// "no exclusion" - used for the very first pick at boot.
size_t pickRandomTrack(size_t excluding) {
  if (TRACK_COUNT <= 1) return 0;
  size_t idx;
  do {
    idx = random(TRACK_COUNT);
  } while (idx == excluding);
  return idx;
}

void playTrack(size_t index) {
  currentTrack = index;
  paused = false;
  player.play(TRACKS[index].fileIndex);
  Animations::onTrackStart();
}

void advanceTrack() {
  if (!RANDOM_MODE) {
    playTrack((currentTrack + 1) % TRACK_COUNT);
    return;
  }
  if (historyPos + 1 < historyLen) {
    historyPos++;
    playTrack(history[historyPos]);
  } else {
    size_t next = pickRandomTrack(currentTrack);
    historyPush(next);
    playTrack(next);
  }
}

void goBackTrack() {
  if (!RANDOM_MODE) {
    playTrack(currentTrack == 0 ? TRACK_COUNT - 1 : currentTrack - 1);
    return;
  }
  if (historyPos > 0) {
    historyPos--;
    playTrack(history[historyPos]);
  }
}

void enterListView() {
  listCursor = currentTrack;
  state = State::LIST_VIEW;
}

void confirmListSelection() {
  if (RANDOM_MODE) {
    historyPush(listCursor);
  }
  playTrack(listCursor);
  state = State::PLAYING;
}

void handleBoot() {
  unsigned long now = millis();
  if (now - lastBootFrameMs >= BOOT_FRAME_INTERVAL_MS) {
    lastBootFrameMs = now;
    bootFrame++;
  }
  display.showBoot(min(bootFrame, (uint8_t)(Display::NUM_BOOT_FRAMES - 1)));

  if (bootFrame < Display::NUM_BOOT_FRAMES) return;

  size_t start = RANDOM_MODE ? pickRandomTrack(TRACK_COUNT) : 0;
  if (RANDOM_MODE) historyPush(start);
  playTrack(start);
  state = State::PLAYING;
}

void handlePlaying() {
  if (btnSelect.longPressEdge()) {
    enterListView();
    return;
  }
  if (btnSelect.shortPress()) {
    paused = !paused;
    if (paused) {
      player.pause();
    } else {
      player.resume();
    }
  }

  if (ANIMATION_DEBUG_MODE) {
    if (btnNext.shortPress()) {
      Animations::setActive((Animations::active() + 1) % ANIMATION_COUNT);
    }
    if (btnPrev.shortPress()) {
      Animations::setActive((Animations::active() + ANIMATION_COUNT - 1) % ANIMATION_COUNT);
    }
  } else {
    if (btnNext.shortPress()) advanceTrack();
    if (btnPrev.shortPress()) goBackTrack();
  }

  unsigned long now = millis();
  if (btnNext.isHeld() && (now - lastNextRepeatMs >= VOLUME_REPEAT_MS)) {
    lastNextRepeatMs = now;
    player.volumeUp();
  }
  if (btnPrev.isHeld() && (now - lastPrevRepeatMs >= VOLUME_REPEAT_MS)) {
    lastPrevRepeatMs = now;
    player.volumeDown();
  }

  if (!paused && player.justFinished()) {
    advanceTrack();
  }

  if (!paused) {
    Animations::tick(now);
  }

  if (now - lastRedrawMs >= REDRAW_INTERVAL_MS) {
    lastRedrawMs = now;
    display.showPlaying(TRACKS[currentTrack], paused);
  }
}

void handleListView() {
  if (btnNext.shortPress()) {
    listCursor = (listCursor + 1) % TRACK_COUNT;
  }
  if (btnPrev.shortPress()) {
    listCursor = (listCursor == 0) ? (TRACK_COUNT - 1) : (listCursor - 1);
  }
  if (btnSelect.shortPress()) {
    confirmListSelection();
    return;
  }

  display.showList(TRACKS, TRACK_COUNT, listCursor);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(34));  // floating ADC pin used only as entropy source

  btnPrev.begin();
  btnSelect.begin();
  btnNext.begin();

  Wire.begin();
  if (!display.begin()) {
    Serial.println("OLED init failed");
  }
  Animations::begin();

  Serial2.begin(9600, SERIAL_8N1, /*rxPin=*/16, /*txPin=*/17);
  playerReady = player.begin(Serial2);
  if (!playerReady) {
    Serial.println("DFPlayer init failed");
    display.showError("DFPlayer no detectado");
  }
}

void loop() {
  btnPrev.update();
  btnSelect.update();
  btnNext.update();

  if (!playerReady) return;

  switch (state) {
    case State::BOOT:
      handleBoot();
      break;
    case State::PLAYING:
      handlePlaying();
      break;
    case State::LIST_VIEW:
      handleListView();
      break;
  }
}
