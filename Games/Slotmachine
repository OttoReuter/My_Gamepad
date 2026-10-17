#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <math.h>

// --- PIN DEFINITIONEN DISPLAY und Tasten
#define TFT_CS   21
#define TFT_RST  20
#define TFT_DC   17
#define TFT_BLK  16

#define NUM_KEYS 8
// up, down, left, right, A, B, C, D
const uint8_t keys[NUM_KEYS] = {4, 5, 3, 2, 7, 6, 9, 8};
bool keyHandled[NUM_KEYS] = {false};

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Offscreen-Puffer pro Walze: reduziert sichtbares Flimmern beim Neuzeichnen.
GFXcanvas16 reelCanvas(75, 120);

#define BLACK       0x0000
#define WHITE       0xFFFF
#define BG_CYAN     0x07FF
#define DEEP_BLUE   0x0018
#define NEON_PINK   0xF81F
#define NEON_BLUE   0x04FF
#define GOLD        0xFDA0
#define ORANGE      0xFD20
#define BRIGHT_RED  0xF800
#define SILVER      0xC618
#define LIGHT_GREY  0xAD55

#define FRUIT_RED    0xF800
#define FRUIT_DARK   0x7800
#define FRUIT_GREEN  0x07E0
//#define FRUIT_GREEN_1  0x566A
#define FRUIT_LEAF   0x3666
#define FRUIT_PURPLE 0x801F
#define FRUIT_LIGHT  0xFD20
#define FRUIT_YELLOW 0xFFE0

// --- Walzenparameter
const int NUM_REELS = 3;
const int SYMBOL_COUNT = 12;       // Symbole pro virtuellem Symbolstreifen
const int SYMBOL_SPACING = 40;     // Abstand der Symbolmittelpunkte
const int REEL_TOP = 100;
const int REEL_HEIGHT = 120;
const int REEL_BOTTOM = REEL_TOP + REEL_HEIGHT;
const int reelX[NUM_REELS] = {77, 160, 242};

// Symbol-IDs: 0 = Kirsche, 1 = Pflaume, 2 = Aprikose
int reelSymbols[NUM_REELS][SYMBOL_COUNT];
int reelHead[NUM_REELS];            // Symbol, das bei Versatz 0 in der Mitte steht
int reelOffset[NUM_REELS] = {0, 0, 0};
int lastShapes[NUM_REELS] = {0, 0, 0};

int lighton = 0;
unsigned long blink = 0;
unsigned long lastPress = 0;
bool umschalt = true;
bool start = false;
bool debounce = false;

unsigned long stake = 0;
unsigned winnings = 0;
bool stakeEntered = false;

// -----------------------------------------------------------------------------
// Fruchtsymbole: bewusst direkt mit Adafruit_GFX gezeichnet, keine Bitmaps nötig
// -----------------------------------------------------------------------------
// ----- Kirsche zeichnen -----
void drawCherry(Adafruit_GFX &gfx, int x, int y) {
  // Stiele
  gfx.drawLine(x - 5, y - 3, x - 1, y - 13, FRUIT_LEAF);
  gfx.drawLine(x - 5, y - 3, x - 1, y - 13, FRUIT_LEAF);
  gfx.drawLine(x - 5, y - 3, x - 1, y - 13, FRUIT_LEAF);
  // Zwei Kirschen mit dunkler Kontur
  gfx.fillCircle(x - 7, y + 5, 9, FRUIT_DARK);
  gfx.fillCircle(x + 8, y + 5, 9, FRUIT_DARK);
  gfx.fillCircle(x - 7, y + 4, 7, FRUIT_RED);
  gfx.fillCircle(x + 8, y + 4, 7, FRUIT_RED);
  // Glanzlichter
  gfx.fillCircle(x - 9, y + 1, 2, WHITE);
  gfx.fillCircle(x + 6, y + 1, 2, WHITE);
  // Kleines Blatt
  gfx.fillTriangle(x - 2, y - 12, x + 8, y - 17, x + 8, y - 9, FRUIT_GREEN);
}

// ----- Pflaume zeichnen -----
void drawPlum(Adafruit_GFX &gfx, int x, int y) {
  // Ovale Pflaume aus zwei überlappenden Rundformen
  gfx.fillRoundRect(x - 12, y - 15, 24, 31, 11, FRUIT_PURPLE);
  gfx.fillCircle(x - 2, y - 5, 9, FRUIT_PURPLE);
  // dunkle Kontur-Andeutung und Pflaumennaht
  gfx.drawLine(x + 1, y - 12, x + 4, y + 11, 0x500F);
  // Stiel und Blatt
  gfx.drawLine(x, y - 14, x + 2, y - 19, 0x7BE0);
  gfx.fillTriangle(x + 1, y - 16, x + 11, y - 20, x + 7, y - 13, FRUIT_GREEN);
  // Glanzlicht
  gfx.drawLine(x - 6, y - 8, x - 7, y - 2, 0xD39F);
  gfx.drawPixel(x - 5, y - 10, WHITE);
}

// ----- Aprikose zeichnen -----
void drawApricot(Adafruit_GFX &gfx, int x, int y) {
  // Orange Frucht mit dunkler Kontur
  gfx.fillCircle(x, y + 1, 15, 0xA280);
  gfx.fillCircle(x, y, 13, FRUIT_LIGHT);
  // leichte Teilung der Frucht
  gfx.drawLine(x + 1, y - 10, x + 3, y + 10, 0xFC00);
  // Blatt und kurzer Stiel
  gfx.drawLine(x, y - 12, x + 2, y - 17, 0x7BE0);
  gfx.fillTriangle(x + 1, y - 15, x + 11, y - 18, x + 7, y - 11, FRUIT_GREEN);
  // Glanzlicht
  gfx.fillCircle(x - 5, y - 5, 3, FRUIT_YELLOW);
  gfx.drawPixel(x - 7, y - 7, WHITE);
}

void drawFruitSymbol(Adafruit_GFX &gfx, int x, int y, int symbolID) {
  if (symbolID == 0) drawCherry(gfx, x, y);
  else if (symbolID == 1) drawPlum(gfx, x, y);
  else drawApricot(gfx, x, y);
}

void drawNeonFrame(int x, int y, int w, int h, int r,
                   uint16_t c1, uint16_t c2, uint16_t c3) {
  tft.drawRoundRect(x, y, w, h, r, c1);
  tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, r, c2);
  tft.drawRoundRect(x + 2, y + 2, w - 4, h - 4, r, c3);
}

void drawBase() {
  tft.fillScreen(GOLD);
  tft.fillRoundRect(5, 5, 310, 230, 20, BLACK);
  drawNeonFrame(10, 10, 300, 220, 20, NEON_BLUE, NEON_PINK, WHITE);
  tft.fillRoundRect(15, 15, 290, 50, 20, DEEP_BLUE);
  
  // Untergrund für Überschrift
  tft.fillRoundRect(22,18, 276,42,10,DEEP_BLUE);

  drawNeonFrame(15, 15, 290, 50, 16, GOLD, ORANGE, GOLD);
  tft.setCursor(63, 30); tft.setTextSize(3);
  tft.setTextColor(BLACK); tft.print("SLOTMACHINE");
  tft.setCursor(61, 28); tft.setTextColor(GOLD); tft.print("SLOTMACHINE");


    
  for (int i = 0; i < 9; i++) {
    tft.fillCircle(40 + (i * 30), 80, 8,
                   (i % 2 == 0 ? ORANGE : BRIGHT_RED));
  }

  drawNeonFrame(30, 95, 260, 130, 8, ORANGE, GOLD, WHITE);

  tft.fillRect(40, REEL_TOP, 75, REEL_HEIGHT, BG_CYAN);
  tft.drawRect(39, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
  tft.fillRect(123, REEL_TOP, 75, REEL_HEIGHT, BG_CYAN);
  tft.drawRect(122, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
  tft.fillRect(205, REEL_TOP, 75, REEL_HEIGHT, BG_CYAN);
  tft.drawRect(203, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
}

// Einen neuen zufälligen Symbolstreifen vorbereiten.
void initializeReels() {
  for (int col = 0; col < NUM_REELS; col++) {
    for (int i = 0; i < SYMBOL_COUNT; i++) {
      reelSymbols[col][i] = random(0, 3);
    }
    reelHead[col] = random(0, SYMBOL_COUNT);
    reelOffset[col] = 0;
    lastShapes[col] = reelSymbols[col][reelHead[col]];
  }
}

// Eine Walze zeichnen. Die Symbole bleiben in fester Reihenfolge und wandern
// gemeinsam nach unten. Der zusätzliche Eintrag oben/unten sorgt für Nachschub.
void drawReel(int col) {
  const int x = reelX[col];

  // Erst im RAM zeichnen. Der Puffer wird anschließend als zusammenhängendes
  // Bild übertragen; das sichtbare Fenster wird nicht vorher leer geblitzt.
  reelCanvas.fillScreen(BG_CYAN);

  for (int n = -1; n <= 3; n++) {
    int symbolIndex = (reelHead[col] + n) % SYMBOL_COUNT;
    if (symbolIndex < 0) symbolIndex += SYMBOL_COUNT;

    int y = 60 + n * SYMBOL_SPACING + reelOffset[col];
    // Der Canvas schneidet überstehende Fruchtteile automatisch ab.
    if (y >= -20 && y <= 140) {
      drawFruitSymbol(reelCanvas, 37, y, reelSymbols[col][symbolIndex]);
    }
  }

  tft.drawRGBBitmap(x - 37, REEL_TOP, reelCanvas.getBuffer(), 75, 120);

  // Fensterkanten nach dem Bildtransfer wieder sichtbar machen.
  if (col == 0) tft.drawRect(39, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
  else if (col == 1) tft.drawRect(122, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
  else tft.drawRect(203, REEL_TOP - 1, 77, REEL_HEIGHT + 2, GOLD);
}

void drawAllReels() {
  for (int col = 0; col < NUM_REELS; col++) drawReel(col);
}

// Pixelweise Bewegung. Bei jeweils 40 Pixeln wird das nächste Symbol zur Mitte.
void advanceReels(int pixels) {
  for (int col = 0; col < NUM_REELS; col++) {
    reelOffset[col] += pixels;
    while (reelOffset[col] >= SYMBOL_SPACING) {
      reelOffset[col] -= SYMBOL_SPACING;
      reelHead[col]--;
      if (reelHead[col] < 0) reelHead[col] = SYMBOL_COUNT - 1;
    }
  }
  drawAllReels();
}

void rollSlots() {
  // Für den bisherigen Programmaufbau bleibt rollSlots() erhalten.
  advanceReels(6);
}

void moveSlots() {
  // Phase 1: zügig beschleunigen
  for (int r = 0; r < 12; r++) {
    advanceReels(4 + r);             // 4 bis 12 Pixel pro Schritt
    delay(61 - (r * 5));             // von 61 auf 1 ms
  }
  
  // Phase 2: längere Fahrt mit höherer Geschwindigkeit
  for (int r = 0; r < 200; r++) {
    rollSlots();                     // 6 Pixel pro Schritt
    delay(1);
  }

  // Phase 3: über mehrere Schritte allmählich abbremsen
  for (int r = 0; r < 18; r++) {
    int step = 12 - (r * 10 / 17);  // 12 bis 2 Pixel
    if (step < 2) step = 2;
    advanceReels(step);
    delay(18 + (r * 12));           // 18 bis 222 ms
  }
  

  // Am Ende exakt auf Symbolmitte ausrichten.
  for (int col = 0; col < NUM_REELS; col++) {
    if (reelOffset[col] != 0) {
      int rest = SYMBOL_SPACING - reelOffset[col];
      while (rest > 0) {
        int step = (rest > 3) ? 3 : rest;
        advanceReels(step);
        rest -= step;
        delay(10);
      }
    }
  }

  for (int col = 0; col < NUM_REELS; col++) {
    lastShapes[col] = reelSymbols[col][reelHead[col]];
  }
}

// ----- Obere Anzeige -----
void drawTopStatus() {
  tft.fillRoundRect(22,18, 276,42,10,DEEP_BLUE);
  tft.setTextSize(2);
  tft.setCursor(25,20); tft.print("stake");
  tft.setCursor(40,40); tft.print(stake);
}

void showWinnings() {
  // Die Gewinnanzeige erscheint erst, nachdem erstmals ein Einsatz gesetzt wurde.
  //if (!stakeEntered) return;
  tft.fillRect(105, 24, 115, 30, DEEP_BLUE);
  tft.setTextSize(2);
  tft.setCursor(120, 20);
  tft.print("credits");
  tft.setCursor(140, 40);
  tft.print(winnings);
}

void showScore() {
  int result = 1;
  if (lastShapes[0] == lastShapes[1] && lastShapes[1] == lastShapes[2]) result = 3;
  else if (lastShapes[0] == lastShapes[1] ||
           lastShapes[1] == lastShapes[2] ||
           lastShapes[0] == lastShapes[2]) result = 2;

  // Gewinnstand aktualisieren: Nur drei gleiche Symbole bringen einen Gewinn.
  if (result == 3) {
    // Auszahlungswert für drei gleiche Früchte.
    if (lastShapes[0] == 0)      winnings += 54; // Kirschen
    else if (lastShapes[0] == 1) winnings += 18; // Pflaumen
    else                         winnings += 30; // Aprikosen
  } else if (winnings > 0) {
    if (result == 1) winnings -= 24;
    if (result == 2) winnings -= 16;
  }

  // Ergebnis rechts anzeigen; die result-Ausgabe bleibt unverändert.
  drawTopStatus();
  tft.setCursor(272, 28);
  tft.setTextColor(GOLD);
  tft.setTextSize(3);
  tft.print(result);

  // Gewinnstand separat in der Mitte anzeigen.
  showWinnings();

  // Lauflicht wieder einschalten
  lighton = 1;
}

// ----- Lauflicht -----
void running_light() {
  if (millis() - blink < 400 && umschalt) {
    for (int i = 0; i < 9; i++) {
      tft.fillCircle(40 + (i * 30), 80, 8,
                     (i % 2 == 0 ? ORANGE : BRIGHT_RED));
    }
  }
  if (millis() - blink < 400 && !umschalt) {
    for (int i = 0; i < 9; i++) {
      tft.fillCircle(40 + (i * 30), 80, 8,
                     (i % 2 == 0 ? BRIGHT_RED : ORANGE));
    }
  }
  if (millis() - blink > 400) {
    umschalt = !umschalt;
    blink = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_RST, OUTPUT); digitalWrite(TFT_RST, LOW); delay(50); digitalWrite(TFT_RST,HIGH);
  tft.init(240, 320); tft.setRotation(3);
  pinMode(TFT_BLK,OUTPUT); digitalWrite(TFT_BLK,HIGH);
  
  for(int i=0;i<NUM_KEYS;i++) pinMode(keys[i],INPUT_PULLUP);

  // Zufallsgenerator initialisieren
  randomSeed(analogRead(28));

  drawBase();
  initializeReels();
  drawAllReels();
  blink = millis();
  lighton = 1;
}

void loop() {
  if (lighton == 1) running_light();

  // ----- Taste UP - stake eingeben ----
  if (!digitalRead(keys[0]) && !debounce) {
    debounce = true; lastPress = millis();
    // ----- Einsatz erhöhen -----
    if (stake < 5) stake += 1;
    winnings = winnings + stake * 18;
    drawTopStatus();
    tft.setCursor(80,35); tft.print("continue - A");
  }
  // ----- Taste DOWN - stake eingeben ----
  if (!digitalRead(keys[1]) && !debounce) {
    debounce = true; lastPress = millis();
    // ----- Einsatz verringern -----
    if (stake > 1) stake -= 1;
    winnings = winnings - stake * 18;
    drawTopStatus();
    tft.setCursor(80,35); tft.print("continue - A");
  }
  
  // ----- Taste LEFT - Einsatz aus einem Gewinn zurückholen -----
  if (!digitalRead(keys[2]) && !debounce) {
    debounce = true; lastPress = millis();
    if (winnings > 90 && stake < 5) {
      stake += 1;
      winnings = winnings - stake * 18;
      //stakeEntered = true;
      drawTopStatus();
      showWinnings();
    }
  }

  // ----- Taste A - Start ----------
  // ------ Walze drehen freigeben --
  if (!digitalRead(keys[4]) && !debounce) {
    debounce = true; lastPress = millis();
    if (stake > 0) {
      start = !start;
      drawTopStatus();
      tft.setCursor(80,35); tft.print("play you game - D");
    }
  }
  
  // ----- Taste D - Walzen drehen -----
  if (!digitalRead(keys[7]) && start && !debounce) {
    debounce = true; lastPress = millis();
    if (stake > 0) {
      // Lauflicht ausschalten
      lighton = 0;
      stake -= 1;
      // ----- Anzeige aktualisieren -----
      drawTopStatus();
      showWinnings();
      // Walzen drehen und Ergebnis anzeigen
      moveSlots();
      showScore();
    }
  }

  // ----- Taste C - Reboot ---------
  if (!digitalRead(keys[6])) {
    rp2040.reboot();
  }

  if (millis() - lastPress > 300) debounce = false;
}
