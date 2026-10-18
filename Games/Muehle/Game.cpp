#include "../include/Game.h"
#include "../include/Display.h"
#include "../include/GameTypes.h"
#include "../include/Muehle.h"

extern Adafruit_ST7789 tft;
extern GameState state;
extern int failed;
extern int menuIndex;
extern int menuIndex;
extern bool isready;

// Belegung der 24 Muehle-Punkte
// 0 = frei, 1 = Human, 2 = Pico
uint8_t board[24] = {0};

extern int humanStones;
extern int picoStones;
extern int humanCapturedRed;
extern int picoCapturedBlack;
extern uint8_t removalTarget;
extern bool movingPhase;
static int selectedSource = -1;
static int selectedDestination = -1;
void drawRemoveCursor(int x, int y);

// Welcher Spieler muss bei einer Wegnahme einen Stein abgeben?
// PICO: Human entfernt einen roten Stein; HUMAN: Human entfernt einen schwarzen Stein.
uint8_t removalTarget = PICO;

// Anzahl der vom jeweiligen Spieler erbeuteten gegnerischen Steine.
int humanCapturedRed = 0;   // Rot, von Human erbeutet (links)
int picoCapturedBlack = 0;  // Schwarz, von Pico erbeutet (rechts)

// Beweglicher Stein
int cursorX = 160;
int cursorY = 120;

// -------------------- Brettgeometrie --------------------
// Brett mittig im Display; Werte fuer 240x320.
// Die Quadrate haben gemeinsame Mittelpunkte an den Seiten.
const int CX = 160;
const int CY = 120;
// Brettzentrum etwas unterhalb der Displaymitte
const int OUTER = 104;       // halbe Seitenlaenge des aeusseren Quadrats
const int MIDDLE = 66;       // halbe Seitenlaenge des mittleren Quadrats
const int INNER = 28;        // halbe Seitenlaenge des inneren Quadrats
const int STONE_R = 5;
const int spacing = 18;
const int startY = (260 - 8 * spacing) / 2;
const int topx = 10;

// Einen neu erbeuteten Stein am unteren Ende der jeweiligen Spalte zeichnen.
void drawCapturedStone(uint8_t removedPlayer) {
    if (removedPlayer == PICO) {
        ++humanCapturedRed;
        int y = startY + (8 - (humanCapturedRed - 1)) * spacing;
        tft.fillCircle(topx + 10, y, 6, ST77XX_RED);
    } else if (removedPlayer == HUMAN) {
        ++picoCapturedBlack;
        int y = startY + (8 - (picoCapturedBlack - 1)) * spacing;
        tft.fillCircle(topx + 285, y, 6, ST77XX_BLACK);
    }
}


Point boardPoints[24] = {
  // Aeusseres Quadrat: Ecken + Seitenmitten
  {CX - OUTER, CY - OUTER}, {CX, CY - OUTER},
  {CX+OUTER, CY-OUTER}, {CX+OUTER, CY},
  {CX+OUTER, CY+OUTER}, {CX, CY+OUTER},
  {CX-OUTER, CY+OUTER}, {CX-OUTER, CY},

  // Mittleres Quadrat
  {CX-MIDDLE, CY-MIDDLE}, {CX, CY-MIDDLE},
  {CX+MIDDLE, CY-MIDDLE}, {CX+MIDDLE, CY},
  {CX+MIDDLE, CY+MIDDLE}, {CX, CY+MIDDLE},
  {CX-MIDDLE, CY+MIDDLE}, {CX-MIDDLE, CY},

  // Inneres Quadrat
  {CX-INNER, CY-INNER}, {CX, CY-INNER},
  {CX+INNER, CY-INNER}, {CX+INNER, CY},
  {CX+INNER, CY+INNER}, {CX, CY+INNER},
  {CX-INNER, CY+INNER}, {CX-INNER, CY}
};

// Linien und weisse Brettpositionen zeichnen
void drawBoardLinesAndPoints(){
    // Drei Quadrate
    tft.drawRect(CX-OUTER, CY-OUTER, 2 * OUTER, 2 * OUTER, ST77XX_BLACK);
    tft.drawRect(CX-MIDDLE, CY-MIDDLE, 2 * MIDDLE, 2 * MIDDLE, ST77XX_BLACK);
    tft.drawRect(CX-INNER, CY-INNER, 2 * INNER, 2 * INNER, ST77XX_BLACK);

    // Verbindungen zwischen den Quadraten
    tft.drawLine(CX, CY-OUTER, CX, CY-INNER, ST77XX_BLACK);
    tft.drawLine(CX+OUTER, CY, CX+INNER, CY, ST77XX_BLACK);
    tft.drawLine(CX, CY+OUTER, CX, CY+INNER, ST77XX_BLACK);
    tft.drawLine(CX-OUTER, CY, CX-INNER, CY, ST77XX_BLACK);

    // 24 Positionsmarkierungen – freie Punkte weiss darstellen.
    for (int i = 0; i < 24; i++) {
        tft.fillCircle(boardPoints[i].x, boardPoints[i].y, STONE_R, ST77XX_WHITE);
    }

    // Bereits gesetzte Steine nach jedem Brett-Redraw wieder einzeichnen.
    for (int i = 0; i < 24; i++) {
        if (board[i] == 1) {
            tft.fillCircle(boardPoints[i].x, boardPoints[i].y, STONE_RADIUS, ST77XX_BLACK);
        } else if (board[i] == 2) {
            tft.fillCircle(boardPoints[i].x, boardPoints[i].y, STONE_RADIUS, ST77XX_RED);
        }
    }
}

void draw_board() {
  // Hintergrund
  tft.fillRect(5, 6, 310, 230, ST77XX_WHITE);
  tft.fillRect(CX-OUTER-12, CY-OUTER-13, 234, 236,ST77XX_BLACK);
  tft.fillRect(CX-OUTER-10, CY-OUTER-10, 230, 230, 0xEF23);
  
  // Linien und Brettpositionen
    drawBoardLinesAndPoints();
}

void drawReserveStones() {
  // Spieler links: 8 Steine in einer Reihe, zentriert
  tft.setCursor(3,25);
  tft.setTextColor(ST77XX_BLACK);
  tft.print("Human");
  for (int i = 0; i < 8; i++) {
    int y = startY + i * spacing;
    tft.fillCircle(topx+10, y, 6, ST77XX_BLACK);
  }
  // Mikrocontroller rechts: 9 Steine in einer Reihe, zentriert
  tft.setCursor(270,25);
  tft.setTextColor(ST77XX_BLACK);
  tft.print("Pico");

  for (int i = 0; i < 9; i++) {
    int y = startY + i * spacing;
    tft.fillCircle(topx+285, y, 6, ST77XX_RED);
  }
}

// ===========================
// Position des Steins bewegen
// ===========================
void select_setposition(int dx, int dy) {
    // Der sichtbare Cursor steht absichtlich 12 Pixel vor dem Zielpunkt.
    // Fuer die Navigation deshalb zuerst den zugehoerigen Brettpunkt als
    // logischen Ausgangspunkt bestimmen. So bleibt die Bewegung auch bei
    // mehreren Schritten auf derselben Geraden stabil.
    int originX = cursorX;
    int originY = cursorY;
    int nearestOrigin = -1;
    int nearestDist2 = 0x7fffffff;

    for (int i = 0; i < 24; ++i) {
        int vx = boardPoints[i].x - cursorX;
        int vy = boardPoints[i].y - cursorY;
        int dist2 = vx * vx + vy * vy;
        if (dist2 < nearestDist2) {
            nearestDist2 = dist2;
            nearestOrigin = i;
        }
    }

    // Nur dann auf einen Brettpunkt einrasten, wenn der Cursor wirklich
    // dessen vorgeschobene Darstellung ist. In der Brettmitte bleibt der
    // bisherige Startpunkt erhalten.
    if (nearestOrigin >= 0 && nearestDist2 <= 20 * 20) {
        originX = boardPoints[nearestOrigin].x;
        originY = boardPoints[nearestOrigin].y;
    }

    int bestPoint = -1;
    int bestScore = 0x7fffffff;
    for (int i = 0; i < 24; ++i) {
        int vx = boardPoints[i].x - originX;
        int vy = boardPoints[i].y - originY;
        int forward = vx * dx + vy * dy;
        if (forward <= 0) continue;

        // Geradeaus liegende Punkte erhalten Vorrang. Der seitliche Versatz
        // wird stark gewichtet, die Entfernung entscheidet bei Gleichstand.
        int side = abs(vx * dy - vy * dx);
        int score = side * 1000 + forward;
        if (score < bestScore) {
            bestScore = score;
            bestPoint = i;
        }
    }
    if (bestPoint < 0) return;

    // Alten Cursor entfernen und Brett neu zeichnen.
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
    drawBoardLinesAndPoints();

    // Wie bisher kurz vor dem Zielpunkt anhalten; D setzt den Stein.
    const int inset = 12;
    int tx = boardPoints[bestPoint].x;
    int ty = boardPoints[bestPoint].y;
    int vx = tx - originX;
    int vy = ty - originY;
    if (abs(vx) >= abs(vy)) {
        cursorX = tx - (vx > 0 ? inset : -inset);
        cursorY = ty;
    } else {
        cursorX = tx;
        cursorY = ty - (vy > 0 ? inset : -inset);
    }
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS, ST77XX_BLACK);
}

// ==========================================
// Cursor zur Steinentfernung bewegen
// ==========================================
void select_remove_position(int dx, int dy) {
    // Den naechsten tatsaechlichen Brettpunkt in Tast Richtung suchen.
    // Vom Brettzentrum aus wird der naechste Punkt in dieser Richtung gewaehlt.
    int bestPoint = -1;
    int bestScore = 0x7fffffff;

    for (int i = 0; i < 24; ++i) {
        int vx = boardPoints[i].x - cursorX;
        int vy = boardPoints[i].y - cursorY;

        // Nur Punkte beruecksichtigen, die in die gedrueckte Richtung liegen.
        int forward = vx * dx + vy * dy;
        if (forward <= 0) continue;

        // Punkte moeglichst in Tast-Richtung bevorzugen; seitlicher
        // Versatz wird staerker gewichtet als die Entfernung.
        int side = abs(vx * dy - vy * dx);
        int score = forward + side * 4;
        if (score < bestScore) {
            bestScore = score;
            bestPoint = i;
        }
    }

    if (bestPoint < 0) return;  // In dieser Richtung kein weiterer Punkt

    // Nur den Brettbereich neu aufbauen. Dadurch verschwindet der alte
    // Ring, ohne den restlichen Bildschirm neu zu zeichnen.
    //tft.fillRect(CX - OUTER - 2, CY - OUTER - 2, 2 * OUTER + 4, 2 * OUTER + 4, 0xEF23);
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
    drawBoardLinesAndPoints();  // Quadrate, Verbindungen, Punkte und Steine

    // Cursor exakt auf den ausgewaehlten Brettpunkt setzen.
    cursorX = boardPoints[bestPoint].x;
    cursorY = boardPoints[bestPoint].y;
    drawRemoveCursor(cursorX, cursorY);
}


static int nearestBoardPoint(int x,int y) {
    int best=-1, dBest=0x7fffffff;
    for(int i=0;i<24;++i){ int dx=x-boardPoints[i].x, dy=y-boardPoints[i].y; int d=dx*dx+dy*dy; if(d<dBest){dBest=d;best=i;} }
    return (dBest<=20*20)?best:-1;
}

void beginMovePhase() {
    movingPhase=true; selectedSource=-1; selectedDestination=-1;
    state=SELECT_ZIEHEN; cursorX=CX; cursorY=CY;
    tft.fillRect(CX-OUTER-2,CY-OUTER-2,2*OUTER+4,2*OUTER+4,0xEF23);
    drawBoardLinesAndPoints(); drawRemoveCursor(cursorX,cursorY);

    // Anzeige: Human ist in der Ziehphase am Zug
    tft.fillRect(65,195,90,20,ST77XX_WHITE);
    tft.setCursor(68,198);
    tft.print("H moves");
}

void select_move_stone(int dx,int dy) {
    int current=nearestBoardPoint(cursorX,cursorY), best=-1, scoreBest=0x7fffffff;
    for(int i=0;i<24;++i) if(board[i]==HUMAN && (current<0 || i!=current)) {
        int vx=boardPoints[i].x-cursorX, vy=boardPoints[i].y-cursorY;
        int forward=vx*dx+vy*dy; if(forward<=0) continue;
        int side=abs(vx*dy-vy*dx); int score=side*1000+forward;
        if(score<scoreBest){scoreBest=score;best=i;}
    }
    if(best<0) return;
    cursorX=boardPoints[best].x; cursorY=boardPoints[best].y;
    tft.fillRect(CX-OUTER-5,CY-OUTER-5,2*OUTER+7,2*OUTER+7,0xEF23);
    //tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
    drawBoardLinesAndPoints(); drawRemoveCursor(cursorX,cursorY);
}

void select_move_destination(int dx,int dy) {
    if(selectedSource<0) return;

    int best=-1, bestScore=0x7fffffff;

    for(int k=0;k<4;++k) {
        // Nachbarpunkte suchen, die frei sind.
        for(int i=0;i<24;++i) {
            if(areAdjacent(selectedSource,i) && board[i]==0) {
                int vx=boardPoints[i].x-boardPoints[selectedSource].x;
                int vy=boardPoints[i].y-boardPoints[selectedSource].y;

                if(vx*dx+vy*dy<=0) continue;

                int score=abs(vx*dy-vy*dx)*1000
                          + abs(vx*dx+vy*dy);

                if(score<bestScore) {
                    bestScore=score;
                    best=i;
                }
            }
        }
        break;
    }

    if(best<0) return;

    selectedDestination=best;
    cursorX=boardPoints[best].x;
    cursorY=boardPoints[best].y;

    // Brett neu zeichnen und den Stein an der Quellposition
    // fuer die optische Vorschau entfernen.
    tft.fillRect(CX-OUTER-5,CY-OUTER-5, 2*OUTER+7,2*OUTER+7,0xEF23);
    drawBoardLinesAndPoints();

    // Quellstein optisch entfernen.
    tft.fillCircle(boardPoints[selectedSource].x, boardPoints[selectedSource].y, STONE_RADIUS+3,0xEF23);
    drawBoardLinesAndPoints();
    tft.fillCircle(boardPoints[selectedSource].x, boardPoints[selectedSource].y, STONE_RADIUS,ST77XX_WHITE);

    // Stein bereits am aktuellen Zielpunkt darstellen.
    tft.fillCircle(cursorX,cursorY, STONE_RADIUS,ST77XX_BLACK);

    // Blauer Ring zeigt weiterhin die aktuelle Auswahl.
    drawRemoveCursor(cursorX,cursorY);
}

bool confirmMoveSelection() {
    int point=nearestBoardPoint(cursorX,cursorY);

    if(state==SELECT_ZIEHEN) {
        if(point<0 || board[point]!=HUMAN ||
           !hasLegalMove(HUMAN)) return false;

        bool can=false;
        for(int i=0;i<24;++i) {
            if(areAdjacent(point,i) && board[i]==0) can=true;
        }
        if(!can) return false;

        selectedSource=point;
        selectedDestination=-1;
        state=SELECT_ZIEL;

        cursorX=boardPoints[point].x;
        cursorY=boardPoints[point].y;
        drawRemoveCursor(cursorX,cursorY);

        return true;
    }

    if(state==SELECT_ZIEL) {
        if(point<0 || point!=selectedDestination ||
           board[point]!=0 ||
           !areAdjacent(selectedSource,point)) return false;

        // Zug jetzt auch logisch auf dem Spielfeld ausführen.
        board[selectedSource]=0;
        board[point]=HUMAN;

        // Brett mit der endgültigen Steinposition neu zeichnen.
        tft.fillRect(CX-OUTER-2,CY-OUTER-2,
                     2*OUTER+4,2*OUTER+4,0xEF23);
        drawBoardLinesAndPoints();

        // Auswahl zurücksetzen.
        selectedSource=-1;
        selectedDestination=-1;

        if(formedMill(point,HUMAN)) {
            // Human hat eine Muehle gebildet.
            // Die Anzeige bleibt bis zum Abschluss der Wegnahme.
            tft.fillRect(65,195,90,20,ST77XX_WHITE);
            tft.setCursor(68,198);
            tft.print("H takes");
            removalTarget=PICO;
            state=REMOVE_STONE;
            cursorX=CX;
            cursorY=CY;
            drawRemoveCursor(cursorX,cursorY);
        } else {
            // Humans Zug ist abgeschlossen.
            tft.fillRect(65,195,90,20,0xEF23);
            performPicoMove();
        }

        return true;
    }

    return false;
}

static int countOnBoard(uint8_t player) {
    int n=0;
    for(int i=0;i<24;++i)
        if(board[i]==player) ++n;
    return n;
}

void performPicoMove() {
    if(countOnBoard(PICO)<=3 || countOnBoard(HUMAN)<=3) { state=GAME_OVER; return; }

    int src = -1;
    int dst = -1;

    // Level 3: erst eine vorhandene Zwickmuehle nutzen, dann eine
    // sofortige Muehle bilden, danach einen Zwickmuehlen-Aufbau verbessern.
    // Level 2 und Level 1 behalten ihre bisherigen Prioritaeten.
    if (level == 3) {
        if (!chooseZwickmuehleMove(PICO, src, dst) &&
            !chooseMillFormingMove(PICO, src, dst) &&
            !chooseZwickmuehleSetupMove(PICO, src, dst)) {
            src = chooseLegalMoveSource(PICO);
            if (src < 0) { state=GAME_OVER; return; }
            dst = chooseLegalMoveDestination(src, PICO);
        }
    } else if (level == 2) {
        if (!chooseMillFormingMove(PICO, src, dst)) {
            src = chooseLegalMoveSource(PICO);
            if (src < 0) { state=GAME_OVER; return; }
            dst = chooseLegalMoveDestination(src,PICO);
        }
    } else {
        src = chooseLegalMoveSource(PICO);
        if (src < 0) { state=GAME_OVER; return; }
        dst = chooseLegalMoveDestination(src,PICO);
    }

    if(dst<0) { state=GAME_OVER; return; }

    cursorX=boardPoints[src].x;
    cursorY=boardPoints[src].y;
    drawRemoveCursor(cursorX,cursorY);
    delay(1000);

    board[src]=0;
    board[dst]=PICO;

    tft.fillRect(CX-OUTER-2,CY-OUTER-2,2*OUTER+4,2*OUTER+4,0xEF23);
    drawBoardLinesAndPoints();

    if(formedMill(dst,PICO)) {
        int target=chooseRemovableStone(HUMAN);
        if(target>=0) {
            cursorX=boardPoints[target].x;
            cursorY=boardPoints[target].y;
            drawRemoveCursor(cursorX,cursorY);
            delay(1000);
            if(removeStone(target,HUMAN)) drawCapturedStone(HUMAN);
        }
    }

    if(countOnBoard(HUMAN)<=3 || countOnBoard(PICO)<=3 ||
       !hasLegalMove(HUMAN) || !hasLegalMove(PICO)) {
        state=GAME_OVER;
        return;
    }

    state=SELECT_ZIEHEN;
    cursorX=CX;
    cursorY=CY;
    drawRemoveCursor(cursorX,cursorY);

    // Anzeige: Human ist wieder am Zug
    tft.fillRect(65,195,90,20,ST77XX_WHITE);
    tft.setCursor(68,198);
    tft.print("H moves");
}

// =======================================
// Roten Stein auf dem Brett setzen
// =======================================
void drawPicoReserve() {
    // Bereich rechts der erneut weiß dargestellt wird nimmt immer um eins ab
    tft.fillRect(278, 6, 38, 6 + (picoStones+3) * spacing, ST77XX_WHITE);
    tft.setCursor(270, 25);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Pico");
    for (int i = 0; i < picoStones; i++) {
        int y = startY + i * spacing;
        tft.fillCircle(topx + 285, y, 6, ST77XX_RED);
    }
}

// Pico setzt in Level 1 weiterhin mit seiner bisherigen einfachen Strategie.
// Level 2 erweitert dieses Verhalten um Verteidigung und bevorzugte Positionen.
void placePicoStone() {
    // Alle drei Level sind spielbar; die jeweilige Strategie wird unten gewaehlt.
    if ((menuIndex < 1 || menuIndex > 3) || picoStones <= 0) return;

    int freePoints[24];
    int freeCount = 0;
    for (int i = 0; i < 24; i++) {
        if (board[i] == 0) freePoints[freeCount++] = i;
    }
    if (freeCount == 0) return;

    int chosen = -1;

    if (level == 3) {
        // Level 3 erbt die Verteidigung und die unmittelbare Muehle aus Level 2.
        chosen = chooseHumanMillBlockingMove();
        if (chosen < 0) chosen = chooseMillCompletingMove(PICO);
        // Nur wenn keine zwingende Aktion ansteht, wird eine konkrete
        // Zwickmuehlen-Vorbereitung gesucht.
        if (chosen < 0) chosen = chooseZwickmuehlePlacement();
        // Die bevorzugten Kreuzungspunkte aus Level 2 bleiben erhalten.
        if (chosen < 0) chosen = chooseLevel2Placement();
    } else if (level == 2) {
        chosen = chooseHumanMillBlockingMove();
        if (chosen < 0) chosen = chooseMillCompletingMove(PICO);
        if (chosen < 0) chosen = chooseLevel2Placement();
    } else {
        // Level 1: unveraendertes bisheriges Verhalten.
        chosen = chooseMillCompletingMove(PICO);
        if (chosen < 0) chosen = freePoints[random(freeCount)];
    }

    if (chosen < 0) return;

    board[chosen] = 2;
    tft.fillCircle(boardPoints[chosen].x, boardPoints[chosen].y,
                   STONE_RADIUS, ST77XX_RED);
    picoStones--;
    drawPicoReserve();

    // Pruefen, ob Pico mit diesem Zug eine Muehle gebildet hat.
    if (formedMill(chosen, PICO)) {
        Serial.println("Pico hat eine Muehle gebildet! Pico waehlt einen erlaubten Human-Stein.");
        tft.fillRect(165,195,90,20,ST77XX_WHITE);
        tft.setCursor(168,198);
        tft.print("P takes");
        delay(2000);
        // Pico waehlt zufaellig einen nach den Muehleregeln entfernbaren
        // schwarzen Stein. Die Auswahl erfolgt automatisch, ohne Human-Eingabe.
        int targetPoint = chooseRemovableStone(HUMAN);
        if (targetPoint >= 0) {
            cursorX = boardPoints[targetPoint].x;
            cursorY = boardPoints[targetPoint].y;
            drawRemoveCursor(cursorX, cursorY);
            Serial.print("Pico entfernt Human-Stein an Punkt ");
            Serial.println(targetPoint);

            // Den blauen Ring eine Sekunde sichtbar lassen.
            delay(1000);
            tft.fillRect(165,195,90,20,0xEF23);

            // Stein entfernen und Brett samt verbleibenden Steinen neu zeichnen.
            if (removeStone(targetPoint, HUMAN)) {
                drawCapturedStone(HUMAN);
                //tft.fillRect(CX - OUTER - 2, CY - OUTER - 2, 2 * OUTER + 4, 2 * OUTER + 4, 0xEF23);
                tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
                drawBoardLinesAndPoints();
            }
        } else {
            Serial.println("Pico-Muehle: Kein entfernbarer Human-Stein vorhanden.");
        }
    }

    // Sind beide Vorräte leer, beginnt nach einer eventuellen Wegnahme die Ziehphase.
    if (humanStones == 0 && picoStones == 0) { beginMovePhase(); return; }

    // Pico ist fertig (mit oder ohne Wegnahme): Human ist wieder an der Reihe.
    state = SELECT_SETPOSITION;
    cursorX = CX;
    cursorY = CY;
    if (humanStones > 0) {
        tft.fillCircle(cursorX, cursorY, STONE_RADIUS, ST77XX_BLACK);
    }
}

// Schwarzen Stein auf einem freien Brettpunkt ablegen.
bool placeHumanStone() {
    int nearest = -1;
    int minDist2 = 0x7fffffff;

    // Zunaechst den geometrisch naechsten Punkt bestimmen – unabhaengig
    // davon, ob er belegt ist. So kann ein belegter Punkt nicht umgangen werden.
    for (int i = 0; i < 24; i++) {
        int dx = cursorX - boardPoints[i].x;
        int dy = cursorY - boardPoints[i].y;
        int dist2 = dx * dx + dy * dy;
        if (dist2 < minDist2) {
            minDist2 = dist2;
            nearest = i;
        }
    }

    // Cursor muss nahe genug an einem Brettpunkt stehen.
    if (nearest < 0 || minDist2 > 20 * 20) return false;

    // Belegungspruefung vor dem Setzen.
    if (board[nearest] != 0) return false;

    // Position speichern: 1 = Human (schwarz).
    board[nearest] = 1;

    // Cursor-Hintergrund wiederherstellen und Brett samt Markierungen neu zeichnen.
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS+3, 0xEF23);
    drawBoardLinesAndPoints();

    cursorX = boardPoints[nearest].x;
    cursorY = boardPoints[nearest].y;
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS, ST77XX_BLACK);

    // Human-Reserve um einen Stein reduzieren und aktualisieren.
    if (humanStones > 0) humanStones--;
    // Bereich links der erneut weiß dargestellt wird nimmt immer um eins ab
    tft.fillRect(5, 6, 40, 6 + (humanStones+2) * spacing, ST77XX_WHITE);
    tft.setCursor(3, 25);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("Human");
    for (int i = 0; i < humanStones-1; i++) {
        int y = startY + i * spacing;
        tft.fillCircle(topx + 10, y, 6, ST77XX_BLACK);
     }

    // Erst nach einem erfolgreichen Human-Zug die Muehle pruefen.
    delay(1000);

    if (formedMill(nearest, HUMAN)) {
        Serial.println("Human hat eine Muehle gebildet!");
        // Mensch muss einen roten Stein entfernen
        tft.fillRect(65,195,90,20,ST77XX_WHITE);
        tft.setCursor(68,198);
        tft.print("H takes");
        state = REMOVE_STONE;
        removalTarget = PICO;
        // Wegnahme-Cursor zunaechst in der Brettmitte anzeigen
        cursorX = CX;
        cursorY = CY;
        drawRemoveCursor(cursorX, cursorY);
    } else {
        // Keine Muehle: Pico ist an der Reihe
        placePicoStone();
    }

    if (state != REMOVE_STONE) {
        cursorX = CX;
        cursorY = CY;

        if (humanStones > 0) {
            tft.fillCircle(cursorX, cursorY, STONE_RADIUS, ST77XX_BLACK);
        }
    }

    return true;
}

// PLATZHALTER: Wird später aufgerufen, wenn Pico eine Mühle hat
bool confirmRemoveStone(int cx, int cy, unsigned char gegner, int &remPoint) {
    remPoint = -1;
    int nearest = -1;
    int minDist2 = 0x7fffffff;

    // Den naechsten der 24 Brettpunkte zur Cursorposition finden.
    for (int i = 0; i < 24; ++i) {
        int dx = cx - boardPoints[i].x;
        int dy = cy - boardPoints[i].y;
        int dist2 = dx * dx + dy * dy;
        if (dist2 < minDist2) {
            minDist2 = dist2;
            nearest = i;
        }
    }

    // Nur eine Auswahl direkt auf einem Brettpunkt akzeptieren.
    if (nearest < 0 || minDist2 > 10 * 10) {
        Serial.println("Wegnahme: Cursor steht auf keinem Brettpunkt.");
        return false;
    }

    if (board[nearest] != gegner) {
        Serial.println("Wegnahme: Auf diesem Punkt steht kein gegnerischer Stein.");
        return false;
    }

    if (!canRemoveStone(nearest, gegner)) {
        Serial.println("Wegnahme: Dieser Stein ist durch eine Muehle geschuetzt.");
        return false;
    }

    if (!removeStone(nearest, gegner)) return false;

    // Die Wegnahme ist erfolgreich: den erbeuteten Stein sofort anzeigen.
    drawCapturedStone(gegner);
    remPoint = nearest;
    return true;
}

// Zeichnet den Cursor für das Steine-Wegnehmen durch Human
void drawRemoveCursor(int cursorX, int cursorY) {
    tft.drawCircle(cursorX, cursorY, STONE_RADIUS+3, ST77XX_BLUE);
    tft.drawCircle(cursorX, cursorY, STONE_RADIUS+2, ST77XX_BLUE);
    tft.drawCircle(cursorX, cursorY, STONE_RADIUS+1, ST77XX_BLUE);
}

// Setzt alle spielbezogenen Werte zurueck (auch beim Ruecksprung ins Menue).
void reset_game_state() {
    for (int i = 0; i < 24; ++i) board[i] = 0;
    humanStones = 9;
    picoStones = 9;
    humanCapturedRed = 0;
    picoCapturedBlack = 0;
    cursorX = CX;
    cursorY = CY;
    removalTarget = PICO;
    movingPhase = false;
    state = MENU;
    mask_verified = false;
}

// ==========================
// Game starten
// ==========================
void start_game() {
    // Neue Partie: alle 24 Brettpositionen sind zunaechst frei.
    for (int i = 0; i < 24; i++) board[i] = 0;
    cursorX = CX;
    cursorY = CY;

    draw_board();
    drawReserveStones();
    // Erster schwarzer Stein: beweglicher Cursor
    tft.fillCircle(cursorX, cursorY, STONE_RADIUS, ST77XX_BLACK);
}