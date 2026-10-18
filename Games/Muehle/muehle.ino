#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "include/Display.h"
#include "include/Game.h"
#include "include/GameTypes.h"
#include "include/Muehle.h"

#define TFT_CS   21
#define TFT_RST  20
#define TFT_DC   17
#define TFT_BLK  16

#define NUM_KEYS 8
// up, down, left, right, A, B, C, D
const uint8_t keys[NUM_KEYS] = {4, 5, 3, 2, 7, 6, 9, 8};
bool keyHandled[NUM_KEYS] = {false};

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ---------------- STATE ----------------
bool debounce = false;
bool mask_verified= false;
bool isready = false;
bool movingPhase = false;
GameState state = MENU;
// Anzahl schwarze und rote Steine
int humanStones = 9;
int picoStones  = 9;
extern int cursorX;
extern int cursorY;
extern uint8_t removalTarget;
void placePicoStone();
void select_move_stone(int dx,int dy);
void select_move_destination(int dx,int dy);
bool confirmMoveSelection();
void performPicoMove();
unsigned long lastPress = 0;
unsigned long startzeit = 0;
unsigned long gemessene_Zeit = 0;
int i;
int j;
int menuIndex = 1;
int level;
String levelstring;


// ===== Setup =====
void setup() {
  Serial.begin(115200);
  pinMode(TFT_RST, OUTPUT); digitalWrite(TFT_RST, LOW); delay(50); digitalWrite(TFT_RST,HIGH);
  tft.init(240, 320); tft.setRotation(3);
  pinMode(TFT_BLK,OUTPUT); digitalWrite(TFT_BLK,HIGH);
  
  for(int i=0;i<NUM_KEYS;i++) pinMode(keys[i],INPUT_PULLUP);

  randomSeed(analogRead(28));
     
  drawMenu();
}

// ================
// ===== Loop =====
// ================
void loop() {
  // ==========
  // Button UP      
  if (!digitalRead(keys[0]) && !debounce) {
    switch(state) {
      case MENU:
        menuIndex--;
        if(menuIndex<1) menuIndex=3;
        drawMenu();
        break;
      case SELECT_SETPOSITION:
        if (humanStones > 0) {
          select_setposition(0, -1);
        }
        break;
      case REMOVE_STONE:
        select_remove_position(0, -1);
        break;
      case SELECT_ZIEHEN: select_move_stone(0,-1); break;
      case SELECT_ZIEL: select_move_destination(0,-1); break;
    }
    debounce = true; lastPress = millis();
  }
  // ===========
  // Button DOWN
  if (!digitalRead(keys[1]) && !debounce) {
    switch(state) {
      case MENU:
        menuIndex++;
        if(menuIndex>3) menuIndex=1;
        drawMenu();
        break;
      case SELECT_SETPOSITION:
        if (humanStones > 0) {
          select_setposition(0, 1);
        }
        break;
      case REMOVE_STONE:
        select_remove_position(0, 1);
        break;
      case SELECT_ZIEHEN: select_move_stone(0,1); break;
      case SELECT_ZIEL: select_move_destination(0,1); break;
    }
    debounce = true; lastPress = millis();
  }
  // ===========
  // Button LEFT
  if (!digitalRead(keys[2]) && !debounce) {
    switch(state) {
      case SELECT_SETPOSITION:
        if (humanStones > 0) {
          select_setposition(-1, 0);
        }
        break;
      case REMOVE_STONE:
        select_remove_position(-1, 0);
        break;
      case SELECT_ZIEHEN: select_move_stone(-1,0); break;
      case SELECT_ZIEL: select_move_destination(-1,0); break;
    }
    debounce = true; lastPress = millis();
  }
  // ===========
  // Button RIGHT
  if (!digitalRead(keys[3]) && !debounce) {
    switch(state) {
      case SELECT_SETPOSITION:
        if (humanStones > 0) {
          select_setposition(1, 0);
        }
        break;
      case REMOVE_STONE:
        select_remove_position(1, 0);
        break;
      case SELECT_ZIEHEN: select_move_stone(1,0); break;
      case SELECT_ZIEL: select_move_destination(1,0); break;
    }
    debounce = true; lastPress = millis();
  }

  // Game starten mit A-Button
  if (!digitalRead(keys[4]) && state == MENU && !debounce) {
    if (!mask_verified) {   // Spiel in der gewählten Schwierigkeit starten
      debounce = true; lastPress = millis();
      tft.fillScreen(ST77XX_RED);
      if (menuIndex == 1) {
        level = 1;
        levelstring = "leicht";
      } else if (menuIndex == 2) {
          level = 2;
          levelstring = "mittel"; ;
      } else if (menuIndex == 3) {
          level = 3;
          levelstring = "schwer";
      }
      startzeit = millis();
      state = SELECT_SETPOSITION;
      start_game();
    }
  }

  // zurueck zum Menu mit B-Button
  if (!digitalRead(keys[5]) && !debounce) {
    // Abbruch und Rückkehr zum Menü
    state = MENU;
    mask_verified = false;
    drawMenu();
    debounce = true; lastPress = millis();
  }

  // C-Button - Reboot (HardwareReset)
  if (!digitalRead(keys[6]) && !debounce) {
    rp2040.reboot();
  }
  
  // Taste D bei Zahlenauswahl gedrueckt
  if(!digitalRead(keys[7]) && !debounce) {
    debounce = true;
    lastPress = millis();

    switch(state) {
      case SELECT_SETPOSITION:
        placeHumanStone();
        break;
      case SELECT_ZIEHEN:
      case SELECT_ZIEL:
        confirmMoveSelection();
        break;
      case REMOVE_STONE: {
        int removedPoint = -1;
        // Je nach Ausloeser der Muehle wird ein roter oder schwarzer Stein entfernt.
        if (confirmRemoveStone(cursorX, cursorY, removalTarget, removedPoint)) {
          if (removalTarget == PICO) {
            Serial.println("Pico-Stein entfernt!");
          } else {
            Serial.println("Human-Stein entfernt!");
          }

          // nach jeder Wegnahme Brettbereich wiederherstellen.
          tft.fillRect(65,195,90,20,0xEF23);
          tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
          // Human hat eine Muehle gebildet und seinen Zug beendet.
          //if (movingPhase && removalTarget == PICO) {
          //  tft.fillRect(165,195,90,20,0xEF23);
          //}
          drawBoardLinesAndPoints();

          // Human ist nach jeder Wegnahme wieder am Zug.
          if (movingPhase) {
            state = SELECT_ZIEHEN; cursorX=CX; cursorY=CY;
            drawRemoveCursor(cursorX,cursorY);
          } else {
            state = SELECT_SETPOSITION; cursorX=CX; cursorY=CY;
            if (humanStones > 0) tft.fillCircle(cursorX,cursorY,STONE_RADIUS,ST77XX_BLACK);
          }

          // Nur wenn Human zuvor eine Muehle gebildet hatte, ist Pico jetzt dran.
          // Bei einer Pico-Muehle hat Pico seinen Zug bereits ausgefuehrt.
          if (removalTarget == PICO) {
            if (movingPhase) performPicoMove(); else placePicoStone();
          }
        }
        break;
      }
    }
  }
  if (state == GAME_OVER) {
    tft.fillRect(70,105,180,35,ST77XX_WHITE); tft.setCursor(80,115); tft.setTextColor(ST77XX_BLACK); tft.setTextSize(2); tft.print("Spiel beendet");
  }
  if (debounce) {
    bool released = true;
    for (int k = 0; k < NUM_KEYS; k++) {
        if (!digitalRead(keys[k])) {
            released = false;
            break;
        }
    }
    if (released && millis() - lastPress > 200) {
        debounce = false;
    }
  }
}
