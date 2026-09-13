#include "animations.h"

namespace {

void drawDiscAnim(Adafruit_SSD1306& oled, uint8_t frame) {
  const int16_t cx = 64, cy = 32, r = 26;
  oled.drawCircle(cx, cy, r, SSD1306_WHITE);
  oled.fillCircle(cx, cy, 3, SSD1306_WHITE);

  float angle = (frame % 16) * (2.0f * PI / 16.0f);
  oled.drawLine(cx, cy, cx + (int16_t)(cosf(angle) * (r - 4)), cy + (int16_t)(sinf(angle) * (r - 4)),
                SSD1306_WHITE);

  for (uint8_t i = 0; i < 8; i++) {
    float a = i * (2.0f * PI / 8.0f);
    oled.drawLine(cx + (int16_t)(cosf(a) * (r - 3)), cy + (int16_t)(sinf(a) * (r - 3)),
                  cx + (int16_t)(cosf(a) * r), cy + (int16_t)(sinf(a) * r), SSD1306_WHITE);
  }
}

void drawBounceAnim(Adafruit_SSD1306& oled, uint8_t frame) {
  constexpr uint8_t STEPS = 24;
  float t = (frame % STEPS) / (float)STEPS;
  float bounce = fabsf(sinf(t * PI));
  int16_t x = 20 + (int16_t)(t * 88);
  int16_t y = 46 - (int16_t)(bounce * 30);

  oled.fillCircle(x, y, 5, SSD1306_WHITE);
  oled.fillRect(x + 4, y - 18, 2, 18, SSD1306_WHITE);
  oled.drawFastHLine(0, 58, 128, SSD1306_WHITE);
}

int8_t activeIndex = -1;
uint8_t currentFrame = 0;
unsigned long lastFrameMs = 0;

}  // namespace

const Animation ANIMATIONS[] = {
    {60, 16, drawDiscAnim, nullptr},
    {45, 24, drawBounceAnim, nullptr},
};
const uint8_t ANIMATION_COUNT = sizeof(ANIMATIONS) / sizeof(ANIMATIONS[0]);

namespace Animations {

void begin() {
  switch (ANIMATION_MODE) {
    case AnimationMode::RANDOM:
      activeIndex = (int8_t)random(ANIMATION_COUNT);
      break;
    case AnimationMode::FIXED:
      activeIndex = (int8_t)FIXED_ANIMATION_INDEX;
      break;
    case AnimationMode::NONE:
      activeIndex = -1;
      break;
  }
  currentFrame = 0;
  lastFrameMs = millis();
}

void onTrackStart() {
  if (ANIMATION_MODE == AnimationMode::RANDOM) {
    activeIndex = (int8_t)random(ANIMATION_COUNT);
    currentFrame = 0;
  }
}

void setActive(int8_t index) {
  activeIndex = index;
  currentFrame = 0;
}

int8_t active() { return activeIndex; }

void tick(unsigned long nowMs) {
  if (activeIndex < 0) return;
  const Animation& anim = ANIMATIONS[activeIndex];
  if (nowMs - lastFrameMs >= anim.frameDelayMs) {
    lastFrameMs = nowMs;
    currentFrame = (currentFrame + 1) % anim.frameCount;
  }
}

bool isBitmap() {
  return activeIndex >= 0 && ANIMATIONS[activeIndex].bitmapFrames != nullptr;
}

const uint8_t* bitmapFrame() {
  if (activeIndex < 0) return nullptr;
  const Animation& anim = ANIMATIONS[activeIndex];
  return anim.bitmapFrames ? anim.bitmapFrames[currentFrame] : nullptr;
}

void drawProcedural(Adafruit_SSD1306& oled) {
  if (activeIndex < 0) return;
  const Animation& anim = ANIMATIONS[activeIndex];
  if (anim.drawProcedural) anim.drawProcedural(oled, currentFrame);
}

}  // namespace Animations
