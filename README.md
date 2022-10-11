<h1>O-Phase - Algorithmus zur Gruppeneinteilung</h1>

Implementiert einen Algorithmus zur Zuteilung von Erstis zu O-Phasen-Gruppen, wobei jeder Ersti die Gruppen bewerten kann.
Der Algorithmus basiert auf Maximum Matching und sollte im Normalfall eine nahezu optimale Lösung liefern - natürlich nur im Rahmen der spezifizierten Parameter.

Build
-----------

Als Buildsystem wird CMake benutzt.

1. Build directory erzeugen: `mkdir build && cd build`
2. cmake ausführen: `cmake .. -DCMAKE_BUILD_TYPE=Release` (beim ersten Ausführen: beinhaltet einen Download der Boost-Dependency)
3. make ausführen: `make GroupAssignment`

Benutzung
-----------

Für eine Übersicht über alle verfügbaren Parameter:

```./GroupAssignment```

Grundlegende Benutzung:

```./GroupAssignment -i <Eingabedatei> -o <Ausgabedatei> [-g <Ausgabeordner>] [-c <Config>] [-t <Typen-Datei>] [Optionen]```


Für die Ein- und Ausgabe wird JSON verwendet.
 - Eingabe [`-i`]: Siehe `input_definition.json`. Welche exakte Variante akzeptiert wird (Student- vs Team-basiert, Rating-Format) kann über entsprechende Paramter spezifiziert werden
 - Ausgabe [`-o`]: Siehe `output_definition.json`. Varianten sind ebenfalls per Parameter spezifizierbar
 - Ergebnis-Übersicht [`-g`]: Legt im angegeben Ordner Dateien mit einer (human-readable) Zusammenfassung der Ergebnisse pro Gruppe an

 Die Implementierung ist darauf ausgelegt, den Fortschritt und eventuelle Probleme live auf der Kommandozeile auszugeben.
 Über `-v` kann hierbei der Detailgrad von 0 (= keine Ausgabe) bis 5 eingestellt werden.

Konfiguration
-----------
Es gibt einige zusätzliche Parameter, um das genaue Verhalten des Algorithmus zu spezifizieren (siehe Hilfetext).
Diese können über die Kommandozeile oder eine Konfigurationsdatei [`-c`] spezifiziert werden, Kommandozeilenparameter überschreiben dabei Werte in der Datei.

Die default-Werte sind eine sinnvolle Baseline, für den realen Einsatz sollte aber definitiv eine vorgefertigte Konfiguration benutzt werden
(etwa `config/config_2022`).

Mindestzahlen für Studi-Typen
-----------
Es ist tendenziell erstrebenswert, dass z.B. Master-Studis gemeinsam in einer Gruppe landen anstatt alleine mit nur Bachelor-Studis.
Hierfür können in der via `-t` angegebenen Datei Minimalzahlen von Studenten eines spezifischen Typs definiert werden.
Der Algorithmus wird versuchen, diese Minimalwerte sicherzustellen - was natürlich nicht in allen Fällen möglich ist.

Jede Zeile der Datei entspricht dabei einem Constraint, das sichergestellt wird.
Beispielsweise gibt die Zeile `mathe bachelor 5` an, dass Mathe-Bachelor-Studis mindestens zu fünft sein sollen (siehe auch die Vorlagen in `config/`).

Es ist stark zu empfehlen, dass die einzelnen Constraints keine Überschneidung haben. Diese könnten wahrscheinlich entweder nicht umgesetzt werden oder führen für manche der betroffenen Studis zu schlechten Ergebnissen (z.B. ist die Ersti-Constraint in `config/types_2021` eher suboptimal).

Kommunikation mit Server
-----------

Für das aktuell verwendete [Frontend](https://gitea.fsmi.uni-karlsruhe.de/o-phase/gruppenzuteilung_gui) liegen unter `data_scripts/` Skripte zum Holen und Senden der Daten an den Server. Die URLs (und ggfs. das Token) sollen in `config.ini` spezifiziert werden und werden dann von den Skripten automatisch genutzt, sofern sie im selben Ordner ausgeführt werden.
 - `./get_data.sh input.json`: holt die Eingabedaten nach `input.json`
 - `./send_results.py output.json`: sendet die in `output.json` bereitgestellte Ausgabe des Algorithmus an den Server

Für das Python-Skript müssen ggfs. noch Dependencies installiert werden, z.B. via `pip install -r data_scripts/requirements.txt`

Tests
-----------
Via `make` werden auch zwei eher grundlegende Tests gebaut, die direkt ausgeführt werden können.

Weiterhin finden sich in `test_data/` anonymisierte Testdaten vergangener Jahre und in `config/` die damals verwendete Konfiguration bzw. Studitypen-Constraints.
Diese sind gut geeignet, um Testläufe direkt auf den alten Daten zu machen.
