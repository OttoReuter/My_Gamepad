#include "../include/Display.h"
#include <Adafruit_ST7789.h>

extern int failed;
extern bool isready;
extern void reset_game_state();

// Clear Screen
void clear_screen() {
  tft.fillScreen(0x4000);
  tft.fillRect(1,1,235,235,0x0320);
}

// ==================================
// ==========Start MENU =============
// ==================================
void drawMenu() {
  reset_game_state();
  tft.fillScreen(0x4000);
  tft.setTextSize(2);
  tft.setCursor(40, 20);
  tft.setTextColor(ST77XX_YELLOW, 0x4000);
  tft.print("Muehle Mini-Console");
  tft.setTextColor(0xC618, 0x4000);
  tft.setCursor(90,180);
  tft.print("C: Reboot");
  tft.setCursor(90,210);
  tft.print("A: Start");
  tft.setCursor(220,170);
  tft.print("D: OK");
  tft.setCursor(220,200);
  tft.print("B: New");
  tft.setTextColor(ST77XX_WHITE, 0x4000);

  drawMenuItem(1, "Level: Easy", 60);
  drawMenuItem(2, "Level: MEDIUM", 100);
  drawMenuItem(3, "Level: HARD", 140);
}

void drawMenuItem(int index, const char* text, int y) {
  if (menuIndex == index) {
    tft.setTextColor(ST77XX_YELLOW, 0x4000);
    tft.setCursor(20,y);tft.print("*");
  } else {
    tft.setTextColor(ST77XX_WHITE, 0x4000);
    tft.setCursor(20,y);tft.print(" ");
  }
  tft.setCursor(60, y);tft.print(text);
}
