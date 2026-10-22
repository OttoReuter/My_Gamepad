#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/bootrom.h"

// --- PIN DEFINITIONEN DISPLAY
#define DISP_DIN   19
#define DISP_CLK   18
#define DISP_CS    21
#define DISP_DC    17
#define DISP_RST   20
#define DISP_BL    16

#define SD_SCK     10
#define SD_MOSI    11
#define SD_MISO    12
#define SD_CS      13

#define BTN_UP     4
#define BTN_DOWN   5
#define BTN_A      7
#define BTN_B      6
#define BTN_C      9

// Instanzen initialisieren (Nutzt Hardware SPI0 für Display, SPI1 für SD)
Adafruit_ST7789 tft = Adafruit_ST7789(DISP_CS, DISP_DC, DISP_RST);

// ===================== STATE =====================
int listIndex = 0;
int anzahl = 0;
bool debounce = false;
bool present = true;
unsigned long lastPress = 0;
String* Liste = nullptr;       // Dynamische Liste aller UF2-Dateien
const int SICHTBARE_DATEIEN = 7;
int ersterSichtbarerEintrag = 0;  // Index der obersten angezeigten Datei
String filename;

void searchSD() {
  if (!SD.begin(SD_CS, SPI_QUARTER_SPEED, SPI1)) {
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(25, 40);
    tft.print("please insert SD-Card");
    tft.setCursor(25,70);
    tft.print("and press C-Button");
    tft.setCursor(25,100);
    tft.print("or I'm starting the");
    tft.setCursor(25,130);
    tft.print("last Game in 3 s!");
    present = false;
    lastPress = millis();
  } else {
    present = true;
    zeigeDateiliste();
  }
}

void setup() {
  Serial.begin(115200);
  // Taster einrichten (Active Low mit Pullup)
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
    
  pinMode(DISP_BL, OUTPUT);
  digitalWrite(DISP_BL, HIGH); // Backlight an
  tft.init(240, 320);          // Display-Auflösung
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  
  tft.setCursor(60, 10);
  tft.println("RP2350 SD-Loader");

  // SPI für SD-Karte auf alternativem Bus (SPI1) konfigurieren
  SPI1.setRX(SD_MISO);
  SPI1.setTX(SD_MOSI);
  SPI1.setSCK(SD_SCK);
  
  searchSD();

  // --- HARDWARE-FIX FÜR RP2350 SPI-TIMING ---
  // Wir greifen direkt auf das zugrundeliegende Hardware-SPI-Objekt zu 
  // und erzwingen ein stabileres Timing für die Datenübertragung (8 Bit, SPI_MODE0)
  spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
  // -------------------------------------------

}

void zeigeDateiliste() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(20, 10);
  tft.println("Waehle eine .UF2 Datei");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(20, 220);
  tft.println("Datei flashen: A Button");

  // Zuerst die Anzahl der UF2-Dateien auf der SD-Karte ermitteln.
  anzahl = 0;
  File root = SD.open("/");
  if (!root) {
    filename = "";
    return;
  }

  while (File entry = root.openNextFile()) {
    if (!entry.isDirectory() && String(entry.name()).endsWith(".UF2")) {
      anzahl++;
    }
    entry.close();
  }

  // Alte dynamische Liste freigeben und Platz für alle Dateien reservieren.
  delete[] Liste;
  Liste = nullptr;

  if (anzahl <= 0) {
    root.close();
    listIndex = 0;
    ersterSichtbarerEintrag = 0;
    filename = "";
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(20, 80);
    tft.println("Keine UF2-Dateien");
    return;
  }

  Liste = new String[anzahl];

  // Verzeichnis erneut vom Anfang lesen und Dateinamen speichern.
  root.rewindDirectory();
  int i = 0;
  while (File entry = root.openNextFile()) {
    if (!entry.isDirectory() &&
        String(entry.name()).endsWith(".UF2") &&
        i < anzahl) {
      Liste[i++] = entry.name();
    }
    entry.close();
  }
  root.close();

  // Falls die Auswahl nach erneutem Aufruf außerhalb der Liste liegt,
  // auf den ersten Eintrag zurücksetzen.
  if (listIndex < 0 || listIndex >= anzahl) {
    listIndex = 0;
  }

  ersterSichtbarerEintrag = 0;
  filename = Liste[listIndex];
  drawlistItem();
}

void drawlistItem() {
  // Nur den Bereich der Dateiliste löschen; Überschrift und Fußzeile bleiben.
  tft.fillRect(15, 35, 305, 180, ST77XX_BLACK);

  if (anzahl <= 0 || Liste == nullptr) {
    return;
  }

  // Die sichtbare Liste so verschieben, dass die Auswahl darin liegt.
  if (listIndex < ersterSichtbarerEintrag) {
    ersterSichtbarerEintrag = listIndex;
  }
  if (listIndex >= ersterSichtbarerEintrag + SICHTBARE_DATEIEN) {
    ersterSichtbarerEintrag = listIndex - SICHTBARE_DATEIEN + 1;
  }

  // Beim oberen bzw. unteren Rand der Gesamtliste sauber ausrichten.
  int maxErsterEintrag = anzahl > SICHTBARE_DATEIEN
                         ? anzahl - SICHTBARE_DATEIEN
                         : 0;
  if (ersterSichtbarerEintrag > maxErsterEintrag) {
    ersterSichtbarerEintrag = maxErsterEintrag;
  }
  if (ersterSichtbarerEintrag < 0) {
    ersterSichtbarerEintrag = 0;
  }

  // Positionsanzeige aktualisieren.
  tft.fillRect(255, 30, 65, 25, ST77XX_BLACK);
  tft.setCursor(257, 40);
  tft.setTextColor(ST77XX_CYAN);
  tft.print(listIndex + 1);
  tft.print("/");
  tft.print(anzahl);

  // Höchstens sieben Dateinamen anzeigen.
  for (int zeile = 0; zeile < SICHTBARE_DATEIEN; zeile++) {
    int dateiIndex = ersterSichtbarerEintrag + zeile;
    if (dateiIndex >= anzahl) {
      break;
    }

    int yPos = 40 + zeile * 25;
    tft.setCursor(50, yPos);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(Liste[dateiIndex]);

    // Sternchen steht neben dem tatsächlich ausgewählten Eintrag.
    if (dateiIndex == listIndex) {
      tft.setCursor(20, yPos);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("*");
    }
  }

  filename = Liste[listIndex];
}

void flashUF2() {

  // ------------------------------------------------------------
  // RP2350 Gamepad Flash-Bereich
  //
  // Loader:
  //   0x10000000 - 0x100FFFFF
  //
  // Anwendung:
  //   0x10100000 - 0x103FFFFF
  //
  // genutzte Flash-Größe:
  //   4 MB
  // ------------------------------------------------------------

  const uint32_t APP_START      = 0x10100000;
  const uint32_t FLASH_XIP_BASE = 0x10000000;
  const uint32_t FLASH_END_ADDR = 0x10400000;

  // UF2 Magic Numbers
  const uint32_t UF2_MAGIC_START0 = 0x0A324655;
  const uint32_t UF2_MAGIC_START1 = 0x9E5D5157;
  const uint32_t UF2_MAGIC_END    = 0x0AB16F30;

  // RP2350 Family IDs
  const uint32_t RP2350_ARM_S  = 0xE48BFF59;
  const uint32_t RP2350_RISCV  = 0xE48BFF5A;
  const uint32_t RP2350_ARM_NS = 0xE48BFF5B;

  // UF2 flags
  const uint32_t UF2_FLAG_NOT_MAIN_FLASH = 0x00000001;
  const uint32_t UF2_FLAG_FAMILY_ID      = 0x00002000;

  // RP2350 Flash Page
  const uint32_t UF2_PAYLOAD_SIZE = 256;

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("Flashe: ");
  tft.println(filename);
  // Progressbar vorbereiten
  const int BAR_X = 50;
  const int BAR_Y = 90;
  const int BAR_W = 200;
  const int BAR_H = 20;

  tft.drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, ST77XX_WHITE);

  Serial.println();
  Serial.println("========================================");
  Serial.print("Starte Flash: ");
  Serial.println(filename);
  Serial.println("========================================");

  File uf2File = SD.open("/" + filename);

  if (!uf2File) {
    tft.setTextColor(ST77XX_RED);
    tft.println("Fehler beim Oeffnen!");

    Serial.println("FEHLER: Datei konnte nicht geoeffnet werden!");

    return;
  }

  Serial.print("UF2-Datei geoeffnet. Groesse: ");
  Serial.print(uf2File.size());
  Serial.println(" Bytes");

  alignas(4) uint8_t buffer[512];

  int blockCount = 0;
  uint32_t lastErasedSector = 0xFFFFFFFF;

  uint32_t expectedBlocks = 0;

  while (uf2File.available() >= 512) {

    int bytesRead = uf2File.read(buffer, 512);

    if (bytesRead != 512) {
      Serial.println("FEHLER: UF2-Block konnte nicht vollstaendig gelesen werden!");
      break;
    }

    // ----------------------------------------------------------
    // UF2 Header auslesen
    // ----------------------------------------------------------

    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t familyId;
    uint32_t magicEnd;

    memcpy(&magicStart0, buffer + 0,   4);
    memcpy(&magicStart1, buffer + 4,   4);
    memcpy(&flags,       buffer + 8,   4);
    memcpy(&targetAddr,  buffer + 12,  4);
    memcpy(&payloadSize, buffer + 16,  4);
    memcpy(&blockNo,     buffer + 20,  4);
    memcpy(&numBlocks,   buffer + 24,  4);
    memcpy(&familyId,    buffer + 28,  4);
    memcpy(&magicEnd,    buffer + 508, 4);

        // ----------------------------------------------------------
    // 1. UF2 Magic prüfen
    // ----------------------------------------------------------

    if (magicStart0 != UF2_MAGIC_START0 ||
        magicStart1 != UF2_MAGIC_START1 ||
        magicEnd    != UF2_MAGIC_END) {
        continue;
    }

    // ----------------------------------------------------------
    // 2. Nicht-Flash-Daten überspringen
    // ----------------------------------------------------------

    if (flags & UF2_FLAG_NOT_MAIN_FLASH) {
      continue;
    }

    // ----------------------------------------------------------
    // 3. Family-ID muss vorhanden sein
    // ----------------------------------------------------------

    if (!(flags & UF2_FLAG_FAMILY_ID)) {
      continue;
    }

    // ----------------------------------------------------------
    // 4. RP2350 Family-ID prüfen
    // ----------------------------------------------------------

    if (familyId != RP2350_ARM_S &&
        familyId != RP2350_RISCV &&
        familyId != RP2350_ARM_NS) {
      continue;
    }

    // ----------------------------------------------------------
    // 5. RP2350 verlangt Payload Size = 256
    // ----------------------------------------------------------

    if (payloadSize != UF2_PAYLOAD_SIZE) {
      continue;
    }

    // ----------------------------------------------------------
    // 6. Zieladresse prüfen
    // UF2 enthält absolute XIP-Adressen.
    // flash_range_program() benötigt dagegen den Offset
    // innerhalb des Flash.
    // ----------------------------------------------------------

    if (targetAddr < APP_START ||
      targetAddr + UF2_PAYLOAD_SIZE > FLASH_END_ADDR) {
      continue;
    }
    
    // ----------------------------------------------------------
    // 7. Flash Offset berechnen
    // 0x10100000 - 0x10000000 = 0x00100000
    // Das ist der physische Flash-Offset.
    // ----------------------------------------------------------

    uint32_t flash_target_offset = targetAddr - FLASH_XIP_BASE;
    
    // Sicherheitsprüfung Page Alignment
    if (flash_target_offset & 0xFF) {
      continue;
    }

    // ----------------------------------------------------------
    // 8. Sektor löschen
    // Ein Flash-Sektor = 4096 Bytes.
    // Jeder Sektor wird innerhalb eines Flash-Vorgangs
    // genau einmal gelöscht.
    // ----------------------------------------------------------

    uint32_t currentSectorAddr =
        flash_target_offset & ~(FLASH_SECTOR_SIZE - 1);

    if (currentSectorAddr != lastErasedSector) {

      uint32_t ints = save_and_disable_interrupts();

      flash_range_erase(
          currentSectorAddr,
          FLASH_SECTOR_SIZE
      );

      restore_interrupts(ints);

      lastErasedSector = currentSectorAddr;
    }

    // ----------------------------------------------------------
    // 9. Payload-Adresse
    // UF2:
    // Byte 0 ... 31   = Header
    // Byte 32 ... 287 = 256 Byte Nutzdaten
    // ----------------------------------------------------------

    const uint8_t *payload = buffer + 32;

    // ----------------------------------------------------------
    // 10. Flash Page programmieren
    // WICHTIG:
    // exakt 256 Bytes!
    // ----------------------------------------------------------

    uint32_t ints = save_and_disable_interrupts();

    flash_range_program(
        flash_target_offset,
        payload,
        UF2_PAYLOAD_SIZE
    );

    restore_interrupts(ints);

    blockCount++;

    // Fortschritt berechnen
    if (numBlocks > 0) {

      int progressWidth =
        ((long)blockCount * (BAR_W - 2)) / numBlocks;

    if (progressWidth > BAR_W - 2)
     progressWidth = BAR_W - 2;

      // Balken füllen
      tft.fillRect(
      BAR_X + 1,
      BAR_Y + 1,
      progressWidth,
      BAR_H - 2,
      ST77XX_GREEN
    );

    // Prozentanzeige
    int percent =
      ((long)blockCount * 100) / numBlocks;

    if (percent > 100)
      percent = 100;

      tft.fillRect(110, 120, 60, 25, ST77XX_BLACK);
      tft.setCursor(135, 125);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(percent);
      tft.print(" %");
    }
  }

  uf2File.close();

  // ------------------------------------------------------------
  // Ergebnis
  // ------------------------------------------------------------

  if (blockCount > 0) {

    tft.setTextColor(ST77XX_GREEN);
    tft.println();
    tft.setCursor(20, 120);
    tft.println("Flash OK!");

    Serial.println("FLASH ERFOLGREICH.");
  }
  else {

    tft.setTextColor(ST77XX_RED);
    tft.println();
    tft.setCursor(20, 120);
    tft.println("Flash FEHLER!");

    Serial.println("FEHLER: Kein gueltiger UF2-Block programmiert.");
  }
}


void startAW() {

    const uint32_t APP_VECTOR = 0x10103000;

    uint32_t *vectorTable = (uint32_t *)APP_VECTOR;

    uint32_t appStack = vectorTable[0];
    uint32_t appReset = vectorTable[1];

    noInterrupts();

    // Stack Pointer und Reset Handler der Anwendung übernehmen.

    asm volatile (
        "msr msp, %0     \n"
        "bx  %1          \n"
        :
        : "r" (appStack), "r" (appReset)
        : "memory"
    );

    while (1) {
    }
}

void loop() {
  // --------- letztes Spiel ohne SD Karte starten -----
  if (present == false && (millis() - lastPress > 5000)) {
    startAW();
  }
  // ----------- Taster UP ---------------
  if (digitalRead(BTN_UP) == LOW && !debounce) {
    if (anzahl > 0 && Liste != nullptr) {
      listIndex--;
      if (listIndex < 0) listIndex = anzahl - 1;
      drawlistItem();
      Serial.println(filename);
    }
    debounce = true; lastPress = millis();
  }
  // ------------ Taster DOWN -------------
  if (digitalRead(BTN_DOWN) == LOW && !debounce) {
    if (anzahl > 0 && Liste != nullptr) {
      listIndex++;
      if (listIndex >= anzahl) listIndex = 0;
      drawlistItem();
      Serial.println(filename);
    }
    debounce = true; lastPress = millis();
  }
  // Flashvorgang und Anwendung starten
  if (digitalRead(BTN_A) == LOW && present == true && !debounce) {
    flashUF2();
    startAW();
  }
  // -------------- RP2350 Reboot ---------
  if (digitalRead(BTN_C) == LOW && !debounce) {
    rp2040.reboot();
  }
  if (millis() - lastPress > 300) debounce = false;
}

