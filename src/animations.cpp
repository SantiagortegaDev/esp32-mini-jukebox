#include "animations.h"
#include "animation_bitmaps.h"

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

static const uint8_t* const bmp0_frames[] = {bmp0_frame0, bmp0_frame1, bmp0_frame2, bmp0_frame3, bmp0_frame4, bmp0_frame5, bmp0_frame6, bmp0_frame7, bmp0_frame8, bmp0_frame9};
static const uint8_t* const bmp1_frames[] = {bmp1_frame0, bmp1_frame1, bmp1_frame2, bmp1_frame3, bmp1_frame4, bmp1_frame5, bmp1_frame6, bmp1_frame7, bmp1_frame8, bmp1_frame9, bmp1_frame10, bmp1_frame11, bmp1_frame12, bmp1_frame13, bmp1_frame14, bmp1_frame15};
static const uint8_t* const bmp2_frames[] = {bmp2_frame0, bmp2_frame1, bmp2_frame2, bmp2_frame3, bmp2_frame4, bmp2_frame5, bmp2_frame6, bmp2_frame7, bmp2_frame8, bmp2_frame9, bmp2_frame10, bmp2_frame11, bmp2_frame12, bmp2_frame13, bmp2_frame14, bmp2_frame15};
static const uint8_t* const bmp3_frames[] = {bmp3_frame0, bmp3_frame1, bmp3_frame2, bmp3_frame3, bmp3_frame4, bmp3_frame5, bmp3_frame6, bmp3_frame7, bmp3_frame8, bmp3_frame9};
static const uint8_t* const bmp4_frames[] = {bmp4_frame0, bmp4_frame1, bmp4_frame2, bmp4_frame3, bmp4_frame4, bmp4_frame5, bmp4_frame6, bmp4_frame7, bmp4_frame8, bmp4_frame9, bmp4_frame10, bmp4_frame11, bmp4_frame12, bmp4_frame13, bmp4_frame14};
static const uint8_t* const bmp5_frames[] = {bmp5_frame0, bmp5_frame1, bmp5_frame2, bmp5_frame3, bmp5_frame4, bmp5_frame5, bmp5_frame6, bmp5_frame7, bmp5_frame8, bmp5_frame9, bmp5_frame10, bmp5_frame11, bmp5_frame12, bmp5_frame13, bmp5_frame14};
static const uint8_t* const bmp6_frames[] = {bmp6_frame0, bmp6_frame1, bmp6_frame2, bmp6_frame3, bmp6_frame4, bmp6_frame5, bmp6_frame6, bmp6_frame7, bmp6_frame8, bmp6_frame9};
static const uint8_t* const bmp7_frames[] = {bmp7_frame0, bmp7_frame1, bmp7_frame2, bmp7_frame3, bmp7_frame4, bmp7_frame5, bmp7_frame6, bmp7_frame7, bmp7_frame8, bmp7_frame9, bmp7_frame10, bmp7_frame11, bmp7_frame12, bmp7_frame13, bmp7_frame14, bmp7_frame15, bmp7_frame16, bmp7_frame17, bmp7_frame18, bmp7_frame19};
static const uint8_t* const bmp8_frames[] = {bmp8_frame0, bmp8_frame1, bmp8_frame2, bmp8_frame3, bmp8_frame4, bmp8_frame5, bmp8_frame6, bmp8_frame7, bmp8_frame8, bmp8_frame9, bmp8_frame10, bmp8_frame11, bmp8_frame12, bmp8_frame13, bmp8_frame14, bmp8_frame15, bmp8_frame16, bmp8_frame17, bmp8_frame18, bmp8_frame19, bmp8_frame20, bmp8_frame21, bmp8_frame22, bmp8_frame23};

const Animation ANIMATIONS[] = {
    {60, 16, drawDiscAnim, nullptr},
    {45, 24, drawBounceAnim, nullptr},
    {80, 10, nullptr, bmp0_frames},
    {80, 16, nullptr, bmp1_frames},
    {80, 16, nullptr, bmp2_frames},
    {80, 10, nullptr, bmp3_frames},
    {80, 15, nullptr, bmp4_frames},
    {80, 15, nullptr, bmp5_frames},
    {80, 10, nullptr, bmp6_frames},
    {80, 20, nullptr, bmp7_frames},
    {80, 24, nullptr, bmp8_frames},
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
