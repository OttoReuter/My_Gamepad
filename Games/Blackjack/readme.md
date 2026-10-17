Die neuen Spiele sind im Zusammenhang mit der Entwicklung des Gamepads und des zugeh&ouml;rigen Bootloaders entstanden. Ich werde einige hier
ausf&uuml;hrlicher vorstellen.<br><br>
<font color=blue>Blackjack</font><br><br>
Programmiert ist die Anwendung mit C++ in der Arduino IDE. Daf&uuml;r bedarf es keiner gro&szlig;en Rechenleistung beim RP2350. Die Spielregeln
entsprechen denen beim Spiel '21'. Ein Spieler tritt gegen den Dealer (Pico) an. Wenn die Taste A gedr&uuml;ckt wird, bekommt der Spieler zwei
Karten, welche auf dem Display angezeigt werden. Der Dealer zieht eine Karte, die ebenfalls sichtbar ist. Nun entscheidet der Spieler, ob er eine
weitere Karte zieht. Dazu dr&uuml;ckt er die Taste D (Hit). Das kann er so lange wiederholen, wie er unter dem Wert 21 bleibt. Wenn der Spieler
keine weitere Karte mehr ziehen will, dr&uuml;ckt er die Taste B (Stand). Danach ist der Dealer an der Reihe. Er versucht den Spieler zu &uuml;berbieten.
Derjenige, der &uuml;ber 21 liegt verliert. Ansonsten ist derjenige der Gewinner, der den h&ouml;heren Zahlenwert hat. Mit dem Dr&uuml;cken der Taste A
beginnt eine neue Runde und bei Taste C wird ein 'Reboot' ausgef&uuml;hrt, was Sie beim Bootloader dann ins Anfangsmenue zur&uuml;ck bringt.
Von dort kann dann ein anderes Spiel von SD-Karte ausgew&auml;hlt und gestartet werden.
