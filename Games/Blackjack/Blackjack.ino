#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <math.h>

// --- PIN DEFINITIONEN DISPLAY und Tasten
#define DISP_DIN   19
#define DISP_CLK   18
#define DISP_CS    21
#define DISP_DC    17
#define DISP_RST   20
#define DISP_BL    16

#define BTN_UP     4
#define BTN_DOWN   5
#define BTN_A      7
#define BTN_B      6
#define BTN_C      9
#define BTN_D      8

// Instanzen initialisieren (Nutzt Hardware SPI0 für Display, SPI1 für SD)
Adafruit_ST7789 tft = Adafruit_ST7789(DISP_CS, DISP_DC, DISP_RST);

bool debounce = false;
unsigned long lastPress = 0;

// Spielvariablen
int playerTotal = 0;
int dealerTotal = 0;
int playerTotalOld = 0;
int dealerTotalOld = 0;
bool gameOver = false;
int lastCard = 0;
int beforeCard = 0;
int first = 0;

bool started = false;

// Farben und einfache, direkt gezeichnete Casino-Grafik
#define FELT_DARK   0x0208
#define FELT_GREEN  0x03A8
#define GOLD        0xFEA0
#define CREAM       0xFFDF
#define CHIP_RED    0xD104
#define CHIP_BLUE   0x239F

void drawChip(int x, int y, int r, uint16_t color) {
  tft.fillCircle(x, y, r, color);
  tft.drawCircle(x, y, r, CREAM);
  tft.drawCircle(x, y, r - 3, CREAM);
  for (int a = 0; a < 360; a += 45) {
    float rad = a * 0.0174533f;
    int x1 = x + (int)((r - 2) * cos(rad));
    int y1 = y + (int)((r - 2) * sin(rad));
    int x2 = x + (int)((r - 6) * cos(rad));
    int y2 = y + (int)((r - 6) * sin(rad));
    tft.drawLine(x1, y1, x2, y2, CREAM);
  }
}

void drawCardBack(int x, int y, int w, int h) {
  tft.fillRoundRect(x + 3, y + 3, w, h, 5, ST77XX_BLACK);
  tft.fillRoundRect(x, y, w, h, 5, CREAM);
  tft.drawRoundRect(x + 4, y + 4, w - 8, h - 8, 4, CHIP_BLUE);
  tft.drawRoundRect(x + 7, y + 7, w - 14, h - 14, 3, CHIP_BLUE);
  for (int yy = y + 10; yy < y + h - 8; yy += 7)
    tft.drawLine(x + 8, yy, x + w - 8, yy, CHIP_BLUE);
}

void drawSuitMark(int x, int y, int size, char suit, uint16_t color) {
  // Kleine, stilisierte Kartensymbole als dezente Hintergrunddekoration
  if (suit == 'D') {
    tft.fillTriangle(x, y - size, x + size, y, x, y + size, ST77XX_BLACK);
    tft.fillTriangle(x, y - size, x - size, y, x, y + size, ST77XX_BLACK);
  } else if (suit == 'H') {
    tft.fillCircle(x - size / 2, y - size / 3, size / 2, CHIP_RED);
    tft.fillCircle(x + size / 2, y - size / 3, size / 2, CHIP_RED);
    tft.fillTriangle(x - size, y - size / 5, x + size, y - size / 5, x, y + size, CHIP_RED);
  } else if (suit == 'S') {
    tft.fillCircle(x - size / 2, y + size / 5, size / 2, ST77XX_GREEN);
    tft.fillCircle(x + size / 2, y + size / 5, size / 2, ST77XX_GREEN);
    tft.fillTriangle(x - size, y + size / 4, x + size, y + size / 4, x, y - size,ST77XX_GREEN );
    tft.fillTriangle(x - size / 3, y + size / 3, x + size / 3, y + size / 3, x, y + size, ST77XX_GREEN);
  } else {
    tft.drawCircle(x, y, size / 2, GOLD);
    tft.drawLine(x - size / 2, y, x + size / 2, y, GOLD);
    tft.drawLine(x, y - size / 2, x, y + size / 2, GOLD);
  }
}

void drawCasinoBackground() {
  tft.fillScreen(FELT_DARK);
  // Filzfläche mit feinen Linien
  for (int y = 8; y < 240; y += 12)
    tft.drawFastHLine(8, y, 304, FELT_GREEN);
  tft.drawRoundRect(5, 5, 310, 230, 12, GOLD);
  tft.drawRoundRect(9, 9, 302, 222, 10, FELT_GREEN);
  // Dezente Symbole am Rand, nicht hinter den Textzeilen
  drawSuitMark(25, 38, 10, 'S', FELT_GREEN);
  drawSuitMark(295, 38, 10, 'H', FELT_GREEN);
  drawSuitMark(25, 207, 10, 'D', FELT_GREEN);
  drawSuitMark(295, 207, 10, 'C', FELT_GREEN);
}

void drawTitleScreen() {
  drawCasinoBackground();
  // Dekorative Karten und Jetons am Rand
  drawCardBack(25, 75, 35, 52);
  drawChip(60, 165, 16, CHIP_RED);
  drawChip(250, 190, 16, CHIP_BLUE);
  tft.setTextWrap(false);
  tft.setTextColor(GOLD);
  tft.setTextSize(3);
  tft.setCursor(85, 35);
  tft.print("BLACKJACK");
  tft.setTextSize(2);
  tft.setTextColor(CREAM);
  tft.setCursor(155, 64);
  tft.print("21");
  tft.drawFastHLine(92, 87, 136, GOLD);
  tft.setTextSize(2);
  tft.setCursor(100,140);
  tft.print("C: New");
  tft.setCursor(100,170);
  tft.print("A: Start");
  tft.setCursor(210,110);
  tft.print("D: Hit");
  tft.setCursor(210,140);
  tft.print("B: Stand");
}

// Funktion um eine zufällige Karte zu ziehen (2 bis 11)
// Ein Ass zählt in dieser vereinfachten Logik als 11 (oder 1 bei Überkauf)
int drawCard() {
  int card = random(2, 12); // Werte von 2 bis 11
  return card;
}

// Hilfsfunktion zur Anpassung des Wertes, falls man über 21 kommt (Ass-Regel)
int checkAce(int total, int newCard) {
  if (total + newCard > 21 && newCard == 11) {
    return 1; // Ass zählt als 1 statt 11
  }
  return newCard;
}

void setup() {
  Serial.begin(115200);
  // Taster einrichten (Active Low mit Pullup)
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(BTN_D, INPUT_PULLUP);
    
  pinMode(DISP_BL, OUTPUT);
  digitalWrite(DISP_BL, HIGH); // Backlight an
  tft.init(240, 320);          // Display-Auflösung
  tft.setRotation(3);

  // Hardware-Zufallsgenerator über ein unbeschaltetes analoges Pin initiieren
  randomSeed(analogRead(26));

  drawTitleScreen();
}

void startNewGame() {
  drawCasinoBackground();
  tft.setTextColor(CREAM);
  tft.setTextSize(3);
  tft.setCursor(20, 95);
  tft.print("Mische Karten...");
  delay(1200);

  // Erste Karten austeilen
  playerTotal = drawCard();
  lastCard = playerTotal;
  
  playerTotal += checkAce(playerTotal, drawCard());
  beforeCard = playerTotal - lastCard;
  dealerTotal = drawCard(); // Sichtbare Karte des Dealers

  gameOver = false;
  updateDisplay();
}


void updateDisplay() {
  drawCasinoBackground();
  tft.setTextWrap(false);
  tft.setTextColor(GOLD);
  tft.setTextSize(3);
  tft.setCursor(50, 50);
  tft.print("Player: ");
  tft.print(playerTotal);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  drawCardBack(238, 34, 35, 52);
  if (first == 0) {
    drawCardBack(275, 45, 35, 52);
    if (beforeCard < 10) {
      tft.setCursor(287,65);
    } else{
      tft.setCursor(280, 65);
    }
    tft.print(beforeCard);
    first++;
  }
  if (lastCard < 10) {
    tft.setCursor(250, 50);
  } else {
    tft.setCursor(243, 50);
  }
  tft.print(lastCard);
  tft.setTextSize(3);

  // Zeile 2: Dealer-Status
  tft.setTextColor(CREAM);
  tft.setCursor(50, 100);
  tft.print("Dealer: ");
  tft.print(dealerTotal);
  tft.setTextColor(CREAM);
  tft.setTextSize(2);
  tft.setCursor(180, 170);
  tft.print("D: Hit");
  tft.setCursor(180, 210);
  tft.print("B: Stand");
  drawSuitMark(258, 112, 12, 'H', CHIP_RED);
}

void dealerTurn() {
  drawCasinoBackground();
  tft.setTextColor(CREAM);
  tft.setTextSize(3);
  tft.setCursor(25, 80);
  tft.print("Dealer zieht...");
  
  // Der Dealer zieht offen weiter, bis er mindestens 17 hat
  dealerTotalOld = dealerTotal;
  dealerTotal += checkAce(dealerTotal, drawCard());

  drawCardBack(100, 155, 35, 52);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  lastCard = dealerTotal - dealerTotalOld;
  tft.setCursor(112, 175);
  if (lastCard > 9) {
    tft.setCursor(105, 175);
  }
  tft.print(lastCard);
  int i = 40;
  delay(1000);
  
  while (dealerTotal < 17) {
    tft.setTextSize(3);
    tft.setTextColor(GOLD);
    tft.setCursor(50, 125);
    tft.print("Dealer: ");
    tft.print(dealerTotal);
    delay(800);  
    
    tft.fillRect(160, 122, 70, 24, FELT_DARK);
    drawCardBack(100+i, 155, 35, 52);
    dealerTotalOld = dealerTotal;
    dealerTotal += checkAce(dealerTotal, drawCard());
    lastCard = dealerTotal - dealerTotalOld;
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(112+i, 175);
    if (lastCard > 9) {
      tft.setCursor(108+i, 175);
    }    
    tft.print(lastCard);
    i += 40;
  }
  tft.setTextSize(3);
  tft.setTextColor(GOLD);
  tft.setCursor(50, 125);
  tft.print("Dealer: ");
  tft.print(dealerTotal);
  delay(2000);
  evaluateWinner();
}

void evaluateWinner() {
  drawCasinoBackground();
  tft.setTextColor(GOLD);
  tft.setTextSize(3);
  tft.setCursor(40, 70);
  tft.print("Dealer:");
  tft.print(dealerTotal);
  tft.setTextColor(CREAM);
  tft.setCursor(40, 110);
  tft.print("vs Player:");
  tft.print(playerTotal);

  tft.setTextColor(GOLD);
  tft.setCursor(40, 160);
  if (dealerTotal > 21) {
    tft.print("Dealer is dead!");
  } else if (playerTotal > dealerTotal) {
    tft.print("You won");
  } else if (playerTotal < dealerTotal) {
    tft.print("You lost!");
  } else {
    tft.print("Unentschieden!");
  }
  started = false;
}

void loop() {
  // Spiel starten
  if (digitalRead(BTN_A) == LOW && !debounce) {
    debounce = true; lastPress = millis();
    started = true;
    gameOver = false;
    first = 0;
    startNewGame();
  }
  // STAND gedrückt? (Spieler bleibt stehen, Dealer ist dran)
  if (digitalRead(BTN_B) == LOW && started && !debounce) {
    debounce = true; lastPress = millis();
    //gameOver = true;
    dealerTurn();
  }
  
  // HIT gedrückt? (Spieler zieht eine Karte)
  if (digitalRead(BTN_D) == LOW && started && !debounce) {
    debounce = true; lastPress = millis();
    playerTotalOld = playerTotal;
    playerTotal += checkAce(playerTotal, drawCard());
    lastCard = playerTotal - playerTotalOld;
    updateDisplay();

    if (playerTotal > 21) {
      drawCasinoBackground();
      tft.setTextColor(GOLD);
      tft.setTextSize(3);
      tft.setCursor(20, 70);
      tft.print("Ueberkauft! (");
      tft.print(playerTotal);
      tft.print(")");
      tft.setTextColor(CREAM);
      tft.setCursor(80, 140);
      tft.print("Verloren!");
      gameOver = true;
      started = false;
    }
  }
  
  // -------------- RP2350 Reboot ---------
  if (digitalRead(BTN_C) == LOW && !debounce) {
    rp2040.reboot();
  }
  if (millis() - lastPress > 300) debounce = false;
}
