#include "display.h"

bool Display::begin() {
  if (!_oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }
  _oled.clearDisplay();
  _oled.display();
  return true;
}

void Display::drawDisc(int16_t cx, int16_t cy, uint8_t frame) {
  const int16_t r = 14;
  _oled.drawCircle(cx, cy, r, SSD1306_WHITE);
  _oled.fillCircle(cx, cy, 2, SSD1306_WHITE);

  float angle = (frame % 16) * (2.0f * PI / 16.0f);
  int16_t x = cx + (int16_t)(cos(angle) * (r - 3));
  int16_t y = cy + (int16_t)(sin(angle) * (r - 3));
  _oled.drawLine(cx, cy, x, y, SSD1306_WHITE);
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

void Display::showPlaying(const Track& track, bool paused, uint8_t frame) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setTextColor(SSD1306_WHITE);

  if (paused) {
    _oled.drawCircle(20, 22, 14, SSD1306_WHITE);
    _oled.fillRect(15, 15, 3, 14, SSD1306_WHITE);
    _oled.fillRect(22, 15, 3, 14, SSD1306_WHITE);
  } else {
    drawDisc(20, 22, frame);
  }

  _oled.setCursor(42, 14);
  _oled.print(track.title);
  _oled.setCursor(42, 26);
  _oled.print(track.author);

  if (paused) {
    _oled.setCursor(0, 54);
    _oled.print("PAUSED");
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
