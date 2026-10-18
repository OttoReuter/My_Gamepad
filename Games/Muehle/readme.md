Die neuen Spiele sind im Zusammenhang mit der Entwicklung des Gamepads und des zugeh&ouml;rigen Bootloaders entstanden. Ich werde einige hier
ausf&uuml;hrlicher vorstellen.<br><br>
<font color=blue>M&uuml;hle</font><br><br>
Sicher kennt es jeder als Brettspiel. Hier ist es in C++ in der Arduino IDE f&uuml;r den Pico programmiert. Daf&uuml;r bedarf es keiner
gro&szlig;en Rechenleistung beim RP2350. Die Spielregeln sind wie beim Brettspiel. Jeder Spieler (hier 'Human' und 'Pico') erh&auml;lt
neun Spielsteine. In der s.g. Setzphase legen die Spieler abwechsend einen Stein auf die wei&szlig;en Felder des Bretts. Wenn ein Spieler
eine M&uuml;hle erreicht (drei Steine in einer Reihe), darf er dem Gegner einen Stein wegnehmen. Dieser darf allerdings nicht in einer
M&uuml;hle sein. Wenn alle Steine gesetzt sind, beginnt die Zugphase. Dabei darf immer um eine Position gezogen werden. Die Zielposition
muss aber frei sein. Ziel ist es neue M&uuml;hlen (oder Zwickm&uuml;hlen) zu bauen um so dem Gegner immer mehr Steine wegzunehmen. In
dieser Version ist das Spiel beendet, wenn ein Spieler nur noch drei Steine hat. Dann hat er verloren.<br><br>
Das Besondere beim M&uuml;hlespiel auf dem Gamepad sind die drei Schwierigkeitsstufen. Normalerweise h&auml;tte ein unge&uuml;bter
Spieler kaum eine Chance gegen die Rechenleistung des Microprozessors. Deshalb 'kennt Pico' in Level 'Easy' gerade mal die Setz- und
Zugregeln und ist dadurch leicht zu besiegen. Anders ist das in den Level 'Medium' und 'Hard'. Hier erkennt Pico die M&ouml;glichkeit
eigene M&uuml;hlen zu bekommen bzw. beim Gegner zu verhindern. Im Level 'Hard' versucht er bewu&szlig;t Zwickm&uuml;hlen zu erreichen
und ist dadurch ein deutlich st&auml;rkerer Gegner.<br><br>
Im Anfangsmenu wird die Funktion der Tasten angezeigt. W&auml;hrend des Spiels zeigt ein schwarzer Kreis, der f&uuml;r 'Human' immer
in der Bildmitte startet, die Position an. Er wird mit den Cursor-Tasten an die gew&uuml;nschte Stelle gesteuert und bleibt immer kurz vor
der angestrebten Position stehen. Mit der Taste D wird der Zug best&auml;tigt, so dass die Position best&auml;tigt wird und der andere
Spieler an der Reihe ist. Da 'Pico' seine Z&uuml;ge sehr schnell ausf&uuml;hrt, wird f&uuml;r 'Human' als Hilfe eine Texteinblendung
('H moves', 'H takes' oder 'P takes') im unteren linken bzw. rechten Bereich angezeigt. So sieht man schnell, was jetzt als n&auml;chstes
dran ist.
