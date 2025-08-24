
QuickLang-GUI (PS Vita)
=======================

Erweiterte Version der QuickLang-App mit:
 - Grafische Oberfläche (vita2d)
 - Favoriten (merken und anzeigen)
 - Konfigurationsdatei: ux0:data/quicklang/config.json
 - Mehrsprachige Labels (UI in English/German)
 - Schnelle Auswahl & Neustart

Voraussetzungen:
 - HENkaku/Ensō (Homebrew)
 - vitaSDK, vita2d (libvita2d) installiert

Build:
```
export VITASDK=/usr/local/vitasdk
export PATH="$VITASDK/bin:$PATH"
mkdir build && cd build
cmake ..
make
# Ergebnis: quicklang.vpk
```

Installation:
 - quicklang.vpk per FTP/USB auf die Vita kopieren und mit VitaShell installieren.
 - Konfig wird in ux0:data/quicklang/config.json gespeichert (falls vorhanden).
