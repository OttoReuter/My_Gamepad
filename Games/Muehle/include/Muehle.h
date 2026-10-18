#pragma once
#include <Arduino.h>
#include "Game.h"

#define SCREEN_W 320
#define SCREEN_H 240

// ==========================================
// Muehle-Regeln fuer das 24-Punkte-Brett
// ==========================================

// Belegung von board[]:
// 0 = frei
// 1 = Human (schwarz)
// 2 = Pico  (rot)

constexpr uint8_t HUMAN = 1;
constexpr uint8_t PICO  = 2;


// ==========================================
// Muehlenpruefung
// ==========================================

// Prueft, ob ein Stein zu einer Muehle gehoert.
bool isMillAt(int punkt, uint8_t spieler);

// Prueft, ob der letzte Zug eine Muehle gebildet hat.
bool formedMill(int letzterPunkt, uint8_t spieler);


// ==========================================
// Wegnahme-Regeln
// ==========================================

// Prueft, ob ein gegnerischer Stein geschlagen
// werden darf.
bool canRemoveStone(int punkt, uint8_t gegner);

// Prueft, ob mindestens ein Stein schlagbar ist.
bool hasRemovableStone(uint8_t gegner);

// Entfernt einen zulaessigen gegnerischen Stein.
bool removeStone(int punkt, uint8_t gegner);

// Waehlt fuer Pico einen zulaessigen Stein aus.
// Rueckgabe -1, wenn kein Stein schlagbar ist.
int chooseRemovableStone(uint8_t gegner);


// ==========================================
// Wegnahme-Cursor
// ==========================================
// Bewegt den Wegnahme-Cursor in die angegebene Richtung.
void select_remove_position(int dx, int dy);

// Zeichnet einen ungefuellten Cursor-Ring
// an der angegebenen Bildschirmposition.
void drawRemoveCursor(int x, int y);

// Prueft die Cursorposition und entfernt bei
// erfolgreicher Auswahl den gegnerischen Stein.
//
// gegner: HUMAN oder PICO
// cursorX, cursorY: aktuelle Bildschirmposition
// removedPoint: Index des entfernten Steins
//
// Rueckgabe:
// true  = Stein erfolgreich entfernt
// false = Auswahl ungueltig
bool confirmRemoveStone(int cursorX, int cursorY, unsigned char gegner, int &removedPoint);
// Gibt einen freien Punkt zur Vervollstaendigung einer Muehle zurueck, sonst -1.
int chooseMillCompletingMove(uint8_t spieler);

// Level-2-Strategie:
// Liefert einen freien Punkt, an dem Pico eine drohende Human-Muehle
// blockieren muss. -1 = keine unmittelbare Drohung.
// Die vier mittleren Kreuzungspunkte werden als bevorzugte
// Setzpositionen verwendet, wenn keine zwingende Aktion vorliegt.
int chooseHumanMillBlockingMove();
int chooseLevel2Placement();

// Sucht in der Ziehphase einen legalen Pico-Zug, der unmittelbar eine
// Muehle bildet. src/dst werden bei Erfolg gefuellt.
bool chooseMillFormingMove(uint8_t player, int &src, int &dst);

// Nachbarschaft und Ziehregeln
bool areAdjacent(int a, int b);
bool hasLegalMove(uint8_t player);
int chooseLegalMoveSource(uint8_t player);
int chooseLegalMoveDestination(int source, uint8_t player);

// Level-3-Zwickmuehlenstrategie.
bool chooseZwickmuehleMove(uint8_t spieler, int &src, int &dst);
bool chooseZwickmuehleSetupMove(uint8_t spieler, int &src, int &dst);
int chooseZwickmuehlePlacement();
