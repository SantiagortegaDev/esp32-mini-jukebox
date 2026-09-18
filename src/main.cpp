#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "animations.h"
#include "buttons.h"
#include "config.h"
#include "display.h"
#include "player.h"
#include "tracks.h"

namespace {

enum class State { BOOT, PLAYING, LIST_VIEW, ANIM_VIEW };

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
uint8_t animHistory[HISTORY_SIZE];  // animation paired with history[i], only meaningful in PREVIEW_MODE
size_t historyLen = 0;
size_t historyPos = 0;

// Preview mode is a scripted intro: after the forced start track (Chirp),
// the very next track it advances to is also forced (Moog City 2) before
// falling back to normal random picks.
bool previewForcedNextPending = false;

uint8_t bootFrame = 0;
unsigned long lastRedrawMs = 0;
unsigned long lastBootFrameMs = 0;
unsigned long lastNextRepeatMs = 0;
unsigned long lastPrevRepeatMs = 0;
unsigned long lastMenuActivityMs = 0;

bool anyButtonEvent() {
  return btnPrev.shortPress() || btnPrev.longPressEdge() || btnSelect.shortPress() ||
         btnSelect.longPressEdge() || btnNext.shortPress() || btnNext.longPressEdge();
}

// Looks up a track by exact title match. Falls back to index 0 (with a
// Serial warning) if PREVIEW_MODE names a title that doesn't exist in
// tracks.h, so a typo there can't hang the firmware.
size_t findTrackByTitle(const char* title) {
  for (size_t i = 0; i < TRACK_COUNT; i++) {
    if (strcmp(TRACKS[i].title, title) == 0) return i;
  }
  Serial.print("PREVIEW_MODE: track not found: ");
  Serial.println(title);
  return 0;
}

void historyPush(size_t trackIndex, uint8_t animIndex) {
  if (historyLen < HISTORY_SIZE) {
    history[historyLen] = trackIndex;
    animHistory[historyLen] = animIndex;
    historyLen++;
    historyPos = historyLen - 1;
    return;
  }
  for (size_t i = 1; i < HISTORY_SIZE; i++) {
    history[i - 1] = history[i];
    animHistory[i - 1] = animHistory[i];
  }
  history[HISTORY_SIZE - 1] = trackIndex;
  animHistory[HISTORY_SIZE - 1] = animIndex;
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

// Mirrors pickRandomTrack for animations: used only in PREVIEW_MODE so every
// track change lands on a different animation than the one currently shown.
uint8_t pickRandomAnimation(int8_t excluding) {
  if (ANIMATION_COUNT <= 1) return 0;
  uint8_t idx;
  do {
    idx = (uint8_t)random(ANIMATION_COUNT);
  } while ((int8_t)idx == excluding);
  return idx;
}

void playTrack(size_t index) {
  currentTrack = index;
  paused = false;
  player.play(TRACKS[index].fileIndex);
}

void advanceTrack() {
  if (!RANDOM_MODE) {
    playTrack((currentTrack + 1) % TRACK_COUNT);
    Animations::onTrackStart();
    return;
  }
  if (historyPos + 1 < historyLen) {
    historyPos++;
    playTrack(history[historyPos]);
    if (PREVIEW_MODE) {
      Animations::setActive(animHistory[historyPos]);
    } else {
      Animations::onTrackStart();
    }
  } else {
    size_t next;
    if (PREVIEW_MODE && previewForcedNextPending) {
      next = findTrackByTitle(PREVIEW_SECOND_TRACK_TITLE);
      previewForcedNextPending = false;
    } else {
      next = pickRandomTrack(currentTrack);
    }
    uint8_t anim = PREVIEW_MODE ? pickRandomAnimation(Animations::active()) : 0;
    historyPush(next, anim);
    playTrack(next);
    if (PREVIEW_MODE) {
      Animations::setActive(anim);
    } else {
      Animations::onTrackStart();
    }
  }
}

void goBackTrack() {
  if (!RANDOM_MODE) {
    playTrack(currentTrack == 0 ? TRACK_COUNT - 1 : currentTrack - 1);
    Animations::onTrackStart();
    return;
  }
  if (historyPos > 0) {
    historyPos--;
    playTrack(history[historyPos]);
    if (PREVIEW_MODE) {
      Animations::setActive(animHistory[historyPos]);
    } else {
      Animations::onTrackStart();
    }
  }
}

void enterListView() {
  listCursor = currentTrack;
  state = State::LIST_VIEW;
  lastMenuActivityMs = millis();
}

void enterAnimView() {
  state = State::ANIM_VIEW;
  lastMenuActivityMs = millis();
}

void confirmListSelection() {
  uint8_t anim = PREVIEW_MODE ? pickRandomAnimation(Animations::active()) : 0;
  if (RANDOM_MODE) {
    historyPush(listCursor, anim);
  }
  playTrack(listCursor);
  if (PREVIEW_MODE) {
    Animations::setActive(anim);
  } else {
    Animations::onTrackStart();
  }
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

  if (PREVIEW_MODE) {
    size_t start = findTrackByTitle(PREVIEW_START_TRACK_TITLE);
    historyPush(start, PREVIEW_START_ANIMATION_INDEX);
    playTrack(start);
    Animations::setActive(PREVIEW_START_ANIMATION_INDEX);
    previewForcedNextPending = true;
    state = State::PLAYING;
    return;
  }

  size_t start = RANDOM_MODE ? pickRandomTrack(TRACK_COUNT) : 0;
  if (RANDOM_MODE) historyPush(start, 0);
  playTrack(start);
  Animations::onTrackStart();
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

  if (btnNext.shortPress()) advanceTrack();
  if (btnPrev.shortPress()) goBackTrack();

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
  if (btnSelect.longPressEdge()) {
    enterAnimView();
    return;
  }
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

void handleAnimView() {
  if (btnSelect.longPressEdge()) {
    enterListView();
    return;
  }
  if (btnSelect.shortPress()) {
    state = State::PLAYING;
    return;
  }
  if (btnNext.shortPress()) {
    Animations::setActive((Animations::active() + 1) % ANIMATION_COUNT);
  }
  if (btnPrev.shortPress()) {
    Animations::setActive((Animations::active() + ANIMATION_COUNT - 1) % ANIMATION_COUNT);
  }

  Animations::tick(millis());
  display.showAnimSelect((uint8_t)Animations::active(), ANIMATION_COUNT);
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
    display.showError();
  }
}

void loop() {
  btnPrev.update();
  btnSelect.update();
  btnNext.update();

  if (!playerReady) return;

  if (state == State::LIST_VIEW || state == State::ANIM_VIEW) {
    if (anyButtonEvent()) {
      lastMenuActivityMs = millis();
    } else if (millis() - lastMenuActivityMs >= MENU_IDLE_TIMEOUT_MS) {
      state = State::PLAYING;
    }
  }

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
    case State::ANIM_VIEW:
      handleAnimView();
      break;
  }
}
