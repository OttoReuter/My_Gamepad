Beim RP2350 unterscheidet sich die Funktionsweise grundlegend von einem PC oder einem Linux-Einplatinencomputer wie dem Raspberry Pi 4/5.
        Da Mikrocontroller Programme direkt aus dem Flash-Speicher ausf&uuml;hren (XIP - Execute in Place) und keinen Standard-SD-Karten-Slot nativ
        im ROM-Bootloader unterst&uuml;tzen, wird f&uuml;r ein solches System ein Custom Bootloader verwendet.<br>
        Die genaue Funktionsweise, wie ein solcher Bootloader Spiele von einer SD-Karte liest und startet, l&auml;uft in folgenden
        Schritten ab:<br><br>
        <font color=blue>1. Das Einschalten und der Sprung in den Custom Bootloader</font><br>
        Wenn das Gamepad eingeschaltet wird, startet als erstes das fest im Chip verankerte Bootrom des RP2350.<br>
        * Dieses Bootrom sucht an einer fest definierten Adresse des eingebauten SPI-Flash-Speichers (ab Adresse 0x10000000) nach ausf&uuml;hrbarem
        Code.<br>
        * Dort ist der Custom Bootloader dauerhaft installiert (er belegt typischerweise die ersten 512 KB des Flash-Speichers). Das Bootrom
        &uuml;bergibt die Kontrolle an diesen Bootloader.<br><br>
        <font color=blue>2. Initialisierung der Hardware und Einlesen der SD-Karte</font><br>
        Sobald der Custom Bootloader l&auml;uft, f&uuml;hrt er elementare Initialisierungen durch:<br>
        * Er startet die Display-Treiber und aktiviert die Steuerungselemente (Tasten).<br>
        * Er initialisiert das SD-Karten-Modul (meist &uuml;ber das SPI-Protokoll).<br>
        * Der Bootloader mountet das Dateisystem der SD-Karte (in der Regel FAT32). Er scannt ein vordefiniertes Verzeichnis (z. B. /games) nach
         ausf&uuml;hrbaren Spieledateien.<br><br>
        <font color=blue>3. Das Boot-Men&uuml; (Grafisches UI)</font><br>
        Auf dem Bildschirm wird dem Nutzer nun ein visuelles Men&uuml; pr&auml;sentiert. Sie k&ouml;nnen durch die Liste der auf der SD-Karte
        abgelegten Spiele bl&auml;ttern. Sobald Sie ein Spiel ausw&auml;hlen und die Start-Taste dr&uuml;cken, leitet der Bootloader den eigentlichen
        Lade- und Flashvorgang ein (Bildbox 1, Bild 2).<br><br>
        <font color=blue>4. Das dynamische "Umflashen"</font><br>
        Da der RP2350 das ausgew&auml;hlte Spiel nicht wie ein PC einfach "per Doppelklick" im Hintergrund ausf&uuml;hren kann, nutzt der Bootloader
        bei meinem Gamepad diese technische Methode, um das Spiel zu starten:<br>
        <font color=blue>Re-Flashing der App-Partition</font><br>
        Der Flash-Speicher bei meinem RP2350 ist folgenderma&szlig;en aufgeteilt: Die ersten 1024 KB geh&ouml;ren dem Bootloader, der Rest bildet die Application
        Partition. Sobald Sie das Spiel w&auml;hlen, liest der Bootloader die entsprechende UF2-Datei blockweise von der SD-Karte und schreibt
        (flasht) sie live in die Application Partition des SPI-Flashs. Dazu muss die Datei nat&uuml;rlich bei Kompilieren in der IDE auf diesen Bereich ab
        der Adresse 0x10100000 gelinkt sein, liegt also genau bei 1 MB. Wenn Sie eine meiner Spieleanwendung mit dem vorgestellten Bootloader verwenden, ist das bereits alles
        erledigt.<br><br>
        <font color=blue>5. Die &Uuml;bergabe an das Spiel (Software Interrupt / Jump)</font><br>
        Nachdem das Spiel vollst&auml;ndig in den Flash-Speicher geschrieben wurde, muss der Bootloader die Ausf&uuml;hrung starten:<br>
        1. Er deinitialisiert die von ihm genutzten Hardware-Peripherien (schaltet z.B. eigene Timer oder Interrupts ab), um dem Spiel
        ein "sauberes" System zu &uuml;bergeben.<br>
        2. Er setzt den Vector Table Pointer (VTOR) der ARM-Cortex-M33-Kerne des RP2350 auf die Startadresse des neuen Spiels um. Dadurch wei&szlig;
        der Prozessor, wo die neuen Befehlsketten liegen.<br>
        3. Er f&uuml;hrt einen gezielten Sprung-Befehl (Jump) zur Einstiegsadresse des Spiels aus.<br>
        Ab dieser Millisekunde l&auml;uft das Spiel autark.
