Die neuen Spiele sind im Zusammenhang mit der Entwicklung des Gamepads und des zugeh&ouml;rigen Bootloaders entstanden. Ich werde einige hier
ausf&uuml;hrlicher vorstellen. Anfangen werde ich mit:<br><br>
<font color=blue>Slotmachine</font><br><br>
Programmiert ist die Anwendung mit C++ in der Arduino IDE. Die Rechenleistung des RP2350 ist mehr als ausreichend, um die Bewegung der sich drehenden
Walzen mit den Fr&uuml;chten fl&uuml;ssig und ruckelfrei darzustellen. Nat&uuml;rlich mussten einige Anpassungen gegen&uuml;ber einem richtigen
Spieleautomaten vorgenommen werden. So gibt es z.B. keinen "Handle" und eine der auf dem Gamepad angeordneten Tasten ist praktisch der "Spin"-Button
(Taste D). Au&szlig;erdem ist die Taste A bei allen neuen Spielen die 'Starttaste' und die Taste C f&uuml;hrt einen Reboot aus. Darauf gehe ich
sp&auml;ter noch genauer ein.<br><br>So spielen Sie auf der Slotmachine:<br>
Mit den Tasten 'UP' und 'DOWN' w&auml;hlen Sie den Einsatz (stake) zwischen 1 und 5. Dabei werden Ihnen Summe &uuml;ber alle(stake * 18)(stake von 1 bis 5)
also 270 credits gutgeschrieben. Mit Taste A schlie&szlig;en Sie die Eingabe ab. Jetzt k&ouml;nnen Sie die Walzen mit dem 'Spin-Button' starten. Je
nach Endstellung erh&ouml;hen sich Ihre Credits oder verringern sich. Bei jedem 'Spin' wird 'stake' um eins verringert. Wenn 'stake' bei Null ist und
gen&uuml;gend Cedits vorhanden sind, kann 'stake' mit der Taste 'LEFT' wieder erh&ouml;ht werden. Da hier nat&uuml;rlich nicht wirklich um Geld
gespielt wird, kann 'stake' auch einfach mit 'UP' wieder aufgef&uuml;llt werden. Mit der Taste C wird ein 'Reboot' ausgef&uuml;hrt, was Sie beim
Bootloader dann ins Anfangsmenue zur&uuml;ck bringt. Von dort kann dann ein anderes Spiel von SD-Karte ausgew&auml;hlt und gestartet werden.
