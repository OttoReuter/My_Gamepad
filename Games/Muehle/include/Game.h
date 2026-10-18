#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "GameTypes.h"

// Stein und Bewegung
#define STONE_RADIUS 6
#define MOVE_STEP    12

bool placeHumanStone();
void select_move_stone(int dx, int dy);
void select_move_destination(int dx, int dy);
bool confirmMoveSelection();
void performPicoMove();
void beginMovePhase();

// Ein Punkt auf dem Muehle-Spielbrett
struct Point {
    int16_t x;
    int16_t y;
};

extern int cursorX;
extern int cursorY;
extern const int CX;
extern const int CY;

extern int i;
extern int j;

extern int solveIndex;
extern int level;
extern String levelstring;

extern bool mask_verified;
extern unsigned long gemessene_Zeit;

// Funktionen
void start_game();
void reset_game_state();
void select_setposition(int dx, int dy);
void select_remove_position(int dx, int dy);

void drawBoardLinesAndPoints();
bool confirmRemoveStone(int cx, int cy, unsigned char player, int &remPoint);
void drawRemoveCursor(int x, int y);

#endif