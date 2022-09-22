<h1>O-Phase - Algorithmus zur Gruppeneinteilung</h1>

Implementiert einen Algorithmus zur Zuteilung von Erstis zu O-Phasen-Gruppen, wobei jeder Ersti die Gruppen bewerten kann.
Der Algorithmus basiert auf Maximum Matching und sollte im Normalfall eine nahezu optimale Lösung liefern - natürlich nur im Rahmen der spezifizierten Parameter.

Build
-----------

Als Buildsystem wird CMake benutzt.

1. Build directory erzeugen: `mkdir build && cd build`
2. cmake ausführen: `cmake ..` (beim ersten Ausführen: beinhaltet einen Download der Boost-Dependency)
3. make ausführen: `make GroupAssignment`

Benutzung
-----------

Für die Ein- und Ausgabe wird JSON verwendet. Für das Format siehe `input_definition.json` bzw. `output_definition.json`.

Grundlegende Benutzung:

```./GroupAssignment -i <Eingabedatei> -o <Ausgabedatei> [-c <Config>] [-g <Ausgabeordner>] [-t <Typen-Datei>]```

Zusätzliche Parameter können über die Kommandozeile oder eine Konfigurationsdatei (übergeben via `-c`) spezifiziert werden.
Über `-g` wird im Zielordner eine human-readable Zusammenfassung des Ergebnisses erzeugt.
Weiterhin können Minimalzahlen von Studenten eines spezifischen Typs definiert werden (in der via `-t` angegebenen Datei).
Der Algorithmus wird versuchen, diese Minimalwerte sicherzustellen - was natürlich nicht in allen Fällen möglich ist.

Für eine Übersicht über alle verfügbaren Parameter:

```./GroupAssignment```
