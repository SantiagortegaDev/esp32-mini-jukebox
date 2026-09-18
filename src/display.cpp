#include "display.h"

#include "animations.h"
#include "boot_bitmap.h"

bool Display::begin() {
  if (!_oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }
  _oled.clearDisplay();
  _oled.display();
  return true;
}

void Display::drawCentered(const char* text, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  _oled.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  _oled.setCursor((128 - (int16_t)w) / 2, y);
  _oled.print(text);
}

void Display::showBoot(uint8_t frame) {
  _oled.clearDisplay();
  if (frame < BOOT_FRAME_COUNT) {
    _oled.drawBitmap(0, 0, BOOT_FRAMES[frame], 128, 64, SSD1306_WHITE);
  }
  _oled.display();
}

void Display::drawActiveAnimation() {
  if (Animations::active() < 0) return;
  if (Animations::isBitmap()) {
    const uint8_t* frame = Animations::bitmapFrame();
    if (frame) _oled.drawBitmap(0, 0, frame, 128, 64, SSD1306_WHITE);
  } else {
    Animations::drawProcedural(_oled);
  }
}

void Display::showPlaying(const Track& track, bool paused) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);

  if (Animations::active() >= 0) {
    drawActiveAnimation();
  } else {
    drawCentered(track.title, 22);
    drawCentered(track.author, 34);
    if (paused) drawCentered("PAUSED", 48);
  }

  _oled.display();
}

void Display::showAnimSelect(uint8_t index, uint8_t count) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);

  drawActiveAnimation();

  char label[16];
  snprintf(label, sizeof(label), "%u/%u", (unsigned)(index + 1), (unsigned)count);
  _oled.fillRect(0, 56, 40, 8, SSD1306_BLACK);
  _oled.setCursor(2, 57);
  _oled.print(label);

  _oled.display();
}

void Display::showList(const Track tracks[], size_t count, size_t cursorIndex) {
  _oled.clearDisplay();
  _oled.setTextSize(1);

  const int8_t visibleRows = 6;
  const int16_t rowHeight = 10;

  int16_t start = (int16_t)cursorIndex - visibleRows / 2;
  if (start < 0) start = 0;
  if (start > (int16_t)count - visibleRows) start = max((int16_t)0, (int16_t)(count - visibleRows));

  for (int8_t i = 0; i < visibleRows && (size_t)(start + i) < count; i++) {
    size_t idx = start + i;
    int16_t y = i * rowHeight;
    bool selected = (idx == cursorIndex);

    if (selected) {
      _oled.fillRect(0, y, 128, rowHeight, SSD1306_WHITE);
      _oled.setTextColor(SSD1306_BLACK);
    } else {
      _oled.setTextColor(SSD1306_WHITE);
    }

    _oled.setCursor(2, y + 1);
    _oled.print(tracks[idx].title);
  }

  _oled.display();
}

// [BEGIN lopaka generated] (this is not ai, is a generator for oled displays)
// better design to the error dfplayer not detected
static const unsigned char PROGMEM image_FaceNopower_bits[] = {0x00,0x00,0x00,0x00,0x03,0x00,0x06,0x00,0x03,0x00,0x06,0x00,0x07,0x00,0x07,0x00,0x7e,0x00,0x03,0xf0,0x7c,0x00,0x01,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xc0,0x00,0x00,0x30,0x60,0x00,0x00,0x7f,0xf0,0x00,0x00,0x7f,0xf0,0x00,0x00,0x70,0x70,0x00,0x00,0x00,0x00,0x00};

void Display::showError() {
    _oled.clearDisplay();
    // FaceNopower
    _oled.drawBitmap(50, 9, image_FaceNopower_bits, 29, 14, 1);
    // string 17
    _oled.setTextColor(1);
    _oled.setTextWrap(false);
    _oled.setCursor(29, 43);
    _oled.print("not detected");
    // string 18
    _oled.setCursor(41, 31);
    _oled.print("DFPlayer");
    _oled.display();
}
// [END lopaka generated]
