#include "display.h"

#include "animations.h"

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
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);

  _oled.setCursor(20, 16);
  _oled.print("MINECRAFT");
  _oled.setCursor(28, 28);
  _oled.print("JUKEBOX");

  int16_t barWidth = map(frame, 0, NUM_BOOT_FRAMES - 1, 0, 108);
  _oled.drawRect(10, 46, 108, 8, SSD1306_WHITE);
  _oled.fillRect(10, 46, barWidth, 8, SSD1306_WHITE);

  _oled.display();
}

void Display::showPlaying(const Track& track, bool paused) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);

  if (Animations::active() >= 0) {
    if (Animations::isBitmap()) {
      const uint8_t* frame = Animations::bitmapFrame();
      if (frame) _oled.drawBitmap(0, 0, frame, 128, 64, SSD1306_WHITE);
    } else {
      Animations::drawProcedural(_oled);
    }
  } else {
    drawCentered(track.title, 22);
    drawCentered(track.author, 34);
    if (paused) drawCentered("PAUSED", 48);
  }

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

void Display::showError(const char* message) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);
  _oled.setCursor(0, 26);
  _oled.println("ERROR:");
  _oled.setCursor(0, 40);
  _oled.println(message);
  _oled.display();
}
