#include "../include/Muehle.h"
//#include "../Game.h"

extern uint8_t board[24];
extern Point boardPoints[24];
extern int level;

// Die 16 Muehlen des Bretts:
// 12 Muehlen auf den drei Quadraten (je 4 Seiten)
// 4 Muehlen auf den Verbindungslinien.
static const uint8_t MILL_LINES[16][3] = {
    // Aeusseres Quadrat
    {0, 1, 2},
    {2, 3, 4},
    {4, 5, 6},
    {6, 7, 0},

    // Mittleres Quadrat
    {8, 9, 10},
    {10, 11, 12},
    {12, 13, 14},
    {14, 15, 8},

    // Inneres Quadrat
    {16, 17, 18},
    {18, 19, 20},
    {20, 21, 22},
    {22, 23, 16},

    // Verbindungslinien: oben, rechts, unten, links
    {1, 9, 17},
    {3, 11, 19},
    {5, 13, 21},
    {7, 15, 23}
};

bool isMillAt(int punkt, uint8_t spieler) {
    if (punkt < 0 || punkt >= 24) return false;
    if (spieler != HUMAN && spieler != PICO) return false;
    if (board[punkt] != spieler) return false;

    for (uint8_t i = 0; i < 16; ++i) {
        bool containsPoint = false;

        for (uint8_t j = 0; j < 3; ++j) {
            if (MILL_LINES[i][j] == punkt) {
                containsPoint = true;
                break;
            }
        }

        if (containsPoint &&
            board[MILL_LINES[i][0]] == spieler &&
            board[MILL_LINES[i][1]] == spieler &&
            board[MILL_LINES[i][2]] == spieler) {
            return true;
        }
    }

    return false;
}

bool formedMill(int letzterPunkt, uint8_t spieler) {
    return isMillAt(letzterPunkt, spieler);
}

bool hasRemovableStone(uint8_t gegner) {
    if (gegner != HUMAN && gegner != PICO) return false;

    // Regel: Wenn mindestens ein gegnerischer Stein ausserhalb einer
    // Muehle steht, duerfen Steine innerhalb einer Muehle nicht geschlagen werden.
    for (int i = 0; i < 24; ++i) {
        if (board[i] == gegner && !isMillAt(i, gegner)) {
            return true;
        }
    }

    // Alle gegnerischen Steine stehen in Muehlen: jeder davon ist erlaubt.
    for (int i = 0; i < 24; ++i) {
        if (board[i] == gegner) return true;
    }

    return false;
}

bool canRemoveStone(int punkt, uint8_t gegner) {
    if (punkt < 0 || punkt >= 24) return false;
    if (gegner != HUMAN && gegner != PICO) return false;
    if (board[punkt] != gegner) return false;

    // Gibt es einen gegnerischen Stein ausserhalb einer Muehle?
    bool hasOutsideMill = false;
    for (int i = 0; i < 24; ++i) {
        if (board[i] == gegner && !isMillAt(i, gegner)) {
            hasOutsideMill = true;
            break;
        }
    }

    // Solange einer ausserhalb steht, darf kein Muehlenstein geschlagen werden.
    if (hasOutsideMill && isMillAt(punkt, gegner)) return false;

    return true;
}

bool removeStone(int punkt, uint8_t gegner) {
    if (!canRemoveStone(punkt, gegner)) return false;
    board[punkt] = 0;
    return true;
}

int chooseRemovableStone(uint8_t gegner) {
    int candidates[24];
    int count = 0;

    for (int i = 0; i < 24; ++i) {
        if (canRemoveStone(i, gegner)) {
            candidates[count++] = i;
        }
    }

    if (count == 0) return -1;
    return candidates[random(count)];
}


int chooseMillCompletingMove(uint8_t spieler) {
    if (spieler != HUMAN && spieler != PICO) return -1;
    for (uint8_t i = 0; i < 16; ++i) {
        int own = 0, empty = -1;
        for (uint8_t j = 0; j < 3; ++j) {
            int p = MILL_LINES[i][j];
            if (board[p] == spieler) ++own;
            else if (board[p] == 0) empty = p;
        }
        if (own == 2 && empty >= 0) return empty;
    }
    return -1;
}




// ============================================================
// Level 2: Setzstrategie
// ============================================================

// Sucht eine unmittelbare Human-Drohung. Eine Drohung liegt vor,
// wenn in einer der 16 Muehlenlinien genau zwei schwarze Steine
// stehen und der dritte Punkt frei ist.
// Gibt es mehrere Drohungen, wird zufaellig eine der verschiedenen
// Blockierpositionen gewaehlt.
int chooseHumanMillBlockingMove() {
    int candidates[24];
    int count = 0;

    for (int i = 0; i < 16; ++i) {
        int human = 0;
        int empty = -1;

        for (int j = 0; j < 3; ++j) {
            int p = MILL_LINES[i][j];
            if (board[p] == HUMAN) {
                ++human;
            } else if (board[p] == 0) {
                empty = p;
            }
        }

        if (human == 2 && empty >= 0) {
            bool alreadyThere = false;
            for (int k = 0; k < count; ++k) {
                if (candidates[k] == empty) {
                    alreadyThere = true;
                    break;
                }
            }
            if (!alreadyThere) candidates[count++] = empty;
        }
    }

    if (count == 0) return -1;
    return candidates[random(count)];
}

// Level 2 bevorzugt die vier Seitenmittelpunkte des mittleren
// Quadrats. Diese Punkte haben jeweils mehrere Anschlussrichtungen
// und sind deshalb fuer die spaetere Ziehphase strategisch interessant.
int chooseLevel2Placement() {
    static const int MIDDLE_CROSSINGS[4] = {9, 11, 13, 15};

    int candidates[4];
    int count = 0;

    for (int i = 0; i < 4; ++i) {
        int p = MIDDLE_CROSSINGS[i];
        if (board[p] == 0) candidates[count++] = p;
    }

    if (count > 0) return candidates[random(count)];

    // Sind alle vier bevorzugten Punkte belegt, wird ein anderer
    // freier Punkt zufaellig gewaehlt.
    int freePoints[24];
    int freeCount = 0;
    for (int i = 0; i < 24; ++i) {
        if (board[i] == 0) freePoints[freeCount++] = i;
    }

    if (freeCount == 0) return -1;
    return freePoints[random(freeCount)];
}

// Alle direkt verbundenen Brettpositionen. -1 beendet die jeweilige Liste.
static const int8_t NEIGHBORS[24][4] = {
 {1,7,-1,-1}, {0,2,9,-1}, {1,3,-1,-1}, {2,4,11,-1},
 {3,5,-1,-1}, {4,6,13,-1}, {5,7,-1,-1}, {6,0,15,-1},
 {9,15,-1,-1}, {8,10,1,17}, {9,11,-1,-1}, {10,12,3,19},
 {11,13,-1,-1}, {12,14,5,21}, {13,15,-1,-1}, {14,8,7,23},
 {17,23,-1,-1}, {16,18,9,-1}, {17,19,-1,-1}, {18,20,11,-1},
 {19,21,-1,-1}, {20,22,13,-1}, {21,23,-1,-1}, {22,16,15,-1}
};


// ============================================================
// Level 2: Ziehstrategie
// ============================================================

// Sucht einen legalen Pico-Zug, mit dem unmittelbar eine Muehle
// entsteht. Die Brettbelegung wird waehrend der Pruefung nur
// temporaer veraendert und danach exakt wiederhergestellt.
bool chooseMillFormingMove(uint8_t player, int &src, int &dst) {
    src = -1;
    dst = -1;

    if (player != HUMAN && player != PICO) return false;

    int sources[24];
    int sourceCount = 0;

    for (int i = 0; i < 24; ++i) {
        if (board[i] != player) continue;

        // Nur Steine mit mindestens einem freien Nachbarpunkt kommen
        // fuer einen normalen Ziehzug in Frage.
        for (int n = 0; n < 4 && NEIGHBORS[i][n] >= 0; ++n) {
            int p = NEIGHBORS[i][n];
            if (board[p] == 0) {
                sources[sourceCount++] = i;
                break;
            }
        }
    }

    if (sourceCount == 0) return false;

    // Alle moeglichen Zuege pruefen. Bei mehreren Muehlenmoeglichkeiten
    // wird zufaellig eine davon ausgewaehlt.
    int foundSrc[24];
    int foundDst[24];
    int foundCount = 0;

    for (int s = 0; s < sourceCount; ++s) {
        int from = sources[s];

        for (int n = 0; n < 4 && NEIGHBORS[from][n] >= 0; ++n) {
            int to = NEIGHBORS[from][n];
            if (board[to] != 0) continue;

            board[from] = 0;
            board[to] = player;

            bool mill = formedMill(to, player);

            board[to] = 0;
            board[from] = player;

            if (mill) {
                foundSrc[foundCount] = from;
                foundDst[foundCount] = to;
                ++foundCount;
            }
        }
    }

    if (foundCount == 0) return false;

    int chosen = random(foundCount);
    src = foundSrc[chosen];
    dst = foundDst[chosen];
    return true;
}



// ============================================================
// Level 3: Zwickmuehlen-Erkennung und begrenzte Vorbereitung
// ============================================================

// Sucht eine einfache Zwickmuehle mit genau fuenf eigenen Steinen:
// Eine Linie ist geschlossen (drei eigene Steine). In einer zweiten,
// disjunkten Muehlenlinie stehen zwei eigene Steine und der dritte Punkt
// ist frei. Der Stein am ersten Linienpunkt kann legal zum freien Punkt
// ziehen und schliesst dort die zweite Muehle.
bool chooseZwickmuehleMove(uint8_t spieler, int &src, int &dst) {
    src = -1;
    dst = -1;
    if (spieler != HUMAN && spieler != PICO) return false;

    int foundSrc[24], foundDst[24], foundCount = 0;
    for (int a = 0; a < 16; ++a) {
        for (int b = 0; b < 16; ++b) {
            if (a == b) continue;
            bool disjoint = true;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    if (MILL_LINES[a][i] == MILL_LINES[b][j]) disjoint = false;
            if (!disjoint) continue; // einfache Zwickmuehle: fuenf verschiedene Steine

            bool firstMill = true;
            for (int i = 0; i < 3; ++i)
                if (board[MILL_LINES[a][i]] != spieler) firstMill = false;
            if (!firstMill) continue;

            for (int k = 0; k < 3; ++k) {
                int to = MILL_LINES[b][k];
                if (board[to] != 0) continue;
                int support = 0;
                for (int j = 0; j < 3; ++j)
                    if (j != k && board[MILL_LINES[b][j]] == spieler) ++support;
                if (support != 2) continue;

                for (int i = 0; i < 3; ++i) {
                    int from = MILL_LINES[a][i];
                    if (!areAdjacent(from, to)) continue;
                    // Nach dem Zug muss die zweite Linie tatsaechlich eine Muehle sein.
                    if (foundCount < 24) {
                        foundSrc[foundCount] = from;
                        foundDst[foundCount] = to;
                        ++foundCount;
                    }
                }
            }
        }
    }
    if (!foundCount) return false;
    int chosen = random(foundCount);
    src = foundSrc[chosen];
    dst = foundDst[chosen];
    return true;
}

// Bewertet, wie weit eine Brettstellung vom Aufbau einer einfachen
// Zwickmuehle entfernt ist. Die Zielstellung benoetigt fuenf eigene
// Steine auf zwei disjunkten Linien; der freie sechste Punkt ist das
// Wechselziel, auf das der Stein aus der geschlossenen Muehle zieht.
static int zwickmuehlePotential(uint8_t spieler) {
    int best = 0;
    uint8_t gegner = (spieler == PICO) ? HUMAN : PICO;
    for (int a = 0; a < 16; ++a) {
        for (int b = 0; b < 16; ++b) {
            if (a == b) continue;
            bool disjoint = true;
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    if (MILL_LINES[a][i] == MILL_LINES[b][j]) disjoint = false;
            if (!disjoint) continue;

            for (int k = 0; k < 3; ++k) {
                int target = MILL_LINES[b][k];
                if (board[target] != 0) continue;
                // Die beiden anderen Punkte der zweiten Linie muessen eigene Steine sein.
                int ownSecond = 0;
                for (int j = 0; j < 3; ++j)
                    if (j != k && board[MILL_LINES[b][j]] == spieler) ++ownSecond;
                if (ownSecond != 2) continue;

                for (int i = 0; i < 3; ++i) {
                    int source = MILL_LINES[a][i];
                    if (!areAdjacent(source, target)) continue;
                    int ownFirst = 0, blocked = 0;
                    for (int j = 0; j < 3; ++j) {
                        int p = MILL_LINES[a][j];
                        if (board[p] == spieler) ++ownFirst;
                        else if (board[p] == gegner) ++blocked;
                    }
                    // Die Quelle ist Teil der ersten Linie. Die Ziel-Linie
                    // hat bereits ihre zwei Stuetzsteine; die erste Linie
                    // ist das noch aufzubauende Element.
                    int score = ownFirst * 10 + ownSecond * 10 - blocked * 18;
                    if (score > best) best = score;
                }
            }
        }
    }
    return best;
}

// Waehlt einen legalen Zug, der den Zwickmuehlen-Aufbau verbessert.
// Es wird nur ein einzelner eigener Zug simuliert; keine tiefe Suche.
bool chooseZwickmuehleSetupMove(uint8_t spieler, int &src, int &dst) {
    src = -1;
    dst = -1;
    int bestScore = 0;
    int bestSrc[24], bestDst[24], bestCount = 0;
    int baseScore = zwickmuehlePotential(spieler);

    for (int from = 0; from < 24; ++from) {
        if (board[from] != spieler) continue;
        for (int n = 0; n < 4 && NEIGHBORS[from][n] >= 0; ++n) {
            int to = NEIGHBORS[from][n];
            if (board[to] != 0) continue;
            board[from] = 0;
            board[to] = spieler;
            int score = zwickmuehlePotential(spieler);
            bool formed = formedMill(to, spieler);
            board[to] = 0;
            board[from] = spieler;

            // Eigene Sofortmuehlen werden separat und hoeher priorisiert.
            if (formed) continue;
            if (score > baseScore && score > bestScore) {
                bestScore = score;
                bestCount = 0;
                bestSrc[bestCount] = from;
                bestDst[bestCount++] = to;
            } else if (score > baseScore && score == bestScore && bestCount < 24) {
                bestSrc[bestCount] = from;
                bestDst[bestCount++] = to;
            }
        }
    }
    if (!bestCount) return false;
    int chosen = random(bestCount);
    src = bestSrc[chosen];
    dst = bestDst[chosen];
    return true;
}

// Bei der Setzphase: bewertet jede freie Position danach, ob sie den
// Aufbau einer Zwickmuehle ermoeglicht. Nur ausreichend konkrete Muster
// (mindestens drei eigene Steine im Zielmuster) werden bevorzugt.
int chooseZwickmuehlePlacement() {
    int candidates[24], scores[24], count = 0, best = 0;
    int base = zwickmuehlePotential(PICO);
    for (int p = 0; p < 24; ++p) {
        if (board[p] != 0) continue;
        board[p] = PICO;
        int score = zwickmuehlePotential(PICO);
        board[p] = 0;
        if (score > base && score >= 30) {
            if (score > best) { best = score; count = 0; }
            if (score == best && count < 24) {
                candidates[count] = p;
                scores[count++] = score;
            }
        }
    }
    (void)scores;
    if (!count) return -1;
    return candidates[random(count)];
}

bool areAdjacent(int a, int b) {
    if (a < 0 || a >= 24 || b < 0 || b >= 24) return false;
    for (int i=0; i<4 && NEIGHBORS[a][i]>=0; ++i)
        if (NEIGHBORS[a][i] == b) return true;
    return false;
}

bool hasLegalMove(uint8_t player) {
    for (int i=0;i<24;++i) if (board[i]==player)
        for (int j=0;j<4 && NEIGHBORS[i][j]>=0;++j)
            if (board[(int)NEIGHBORS[i][j]]==0) return true;
    return false;
}

int chooseLegalMoveSource(uint8_t player) {
    int candidates[24], n=0;
    for (int i=0;i<24;++i) if (board[i]==player) {
        for (int j=0;j<4 && NEIGHBORS[i][j]>=0;++j)
            if (board[(int)NEIGHBORS[i][j]]==0) { candidates[n++]=i; break; }
    }
    return n ? candidates[random(n)] : -1;
}

int chooseLegalMoveDestination(int source, uint8_t player) {
    if (source<0 || source>=24 || board[source]!=player) return -1;
    int candidates[4], n=0;
    for (int j=0;j<4 && NEIGHBORS[source][j]>=0;++j)
        if (board[(int)NEIGHBORS[source][j]]==0) candidates[n++]=NEIGHBORS[source][j];
    return n ? candidates[random(n)] : -1;
}
