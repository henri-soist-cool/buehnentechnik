# Dokumentation: ESP32 HUB75 Matrix-Display mit ESP-NOW

Dieses Projekt nutzt einen **ESP32**, um Informationen auf einem **HUB75 LED-Matrix-Panel (64x32 Pixel)** anzuzeigen. Die Daten (Zahlen von 1 bis 5) werden drahtlos via **ESP-NOW** empfangen.

## 1. Funktionsweise im Überblick

Der Code versetzt den ESP32 in einen Empfangsmodus. Sobald ein anderer ESP32 einen passenden Datensatz sendet, wird eine vordefinierte Nachricht in einer spezifischen Farbe auf dem Display angezeigt. Nach 30 Sekunden ohne neues Signal kehrt das Display automatisch in einen "Bereit"-Zustand (Idle) zurück.

---

## 2. Kernkomponenten des Codes

### Bibliotheken & Konfiguration

* **`ESP32-HUB75-MatrixPanel-I2S-DMA.h`**: Ermöglicht eine flüssige Darstellung auf dem LED-Panel durch Nutzung von DMA (Direct Memory Access), was den Hauptprozessor entlastet.
* **`esp_now.h`**: Ein Protokoll von Espressif für die schnelle, verbindungslose Kommunikation zwischen ESP-Modulen ohne WLAN-Router.

### Datenstruktur

```cpp
typedef struct {
  int number;
} esp_now_message_t;

```

Hier wird festgelegt, wie die "Post" aussieht: Ein einfaches Paket, das eine Ganzzahl (`int`) enthält.

---

## 3. Logik-Tabellen

Je nachdem, welche Zahl (`1-5`) empfangen wird, ändern sich Text und Farbe auf dem Display:

| Zahl | Text | Farbe | Bedeutung (Beispiel) |
| --- | --- | --- | --- |
| **1** | Spot an | Weiß | Beleuchtung Fokus |
| **2** | Licht an | Rot | Allgemeine Beleuchtung |
| **3** | Vorhang | Blau | Bühne öffnen |
| **4** | Mikro an | Grün | Audio aktiv |
| **5** | Fertig | Gelb | Umbau abgeschlossen |
| *Rest* | Bereit | Blau (gedimmt) | Standby-Modus |

---

## 4. Wichtige Funktionen

### `onDataRecv` (Der Empfänger)

Dies ist die "Callback"-Funktion. Sie wird automatisch aufgerufen, sobald Daten per Funk eintreffen:

1. Die Daten werden in die Struktur kopiert.
2. Der Zeitstempel `lastUpdate` wird aktualisiert.
3. Die Helligkeit wird erhöht und die Nachricht mit `drawMessage()` gezeichnet.

### `centerX` (Die Zentrierung)

Da das Display 64 Pixel breit ist, berechnet diese Funktion dynamisch die Startposition für den Text, damit dieser mittig erscheint. Ein Zeichen wird dabei mit einer Breite von 6 Pixeln kalkuliert.

### `showIdleScreen` (Energie sparen & Status)

Wenn 30 Sekunden lang keine Nachricht eingeht, wird das Display dunkler geschaltet (`setBrightness8(50)`) und zeigt lediglich "Bereit" an. Dies schont die LEDs und signalisiert, dass das System aktiv ist.

---

## 5. Setup & Endlosschleife

* **`setup()`**: Initialisiert die serielle Konsole, konfiguriert das HUB75-Panel (speziell für den `FM6124` Treiber-Chip) und startet den ESP-NOW Empfangsmodus. Zudem wird die **MAC-Adresse** ausgegeben – diese ist wichtig, damit der Sender weiß, wohin er die Daten schicken muss.
* **`loop()`**: Prüft kontinuierlich, ob die 30 Sekunden (`timeoutMs`) abgelaufen sind. Wenn ja, wird der Idle-Screen aktiviert.

---

## 6. Hardware-Hinweise

* **Stromversorgung**: Ein 64x32 Panel kann bei voller Helligkeit und weißem Text viel Strom ziehen (mehrere Ampere). Achte auf ein ausreichend starkes 5V Netzteil.
* **Pin-Mapping**: Die Standard-Pins der Library müssen physisch mit dem Flachbandkabel des Panels verbunden sein (OE, CLK, LAT, sowie die RGB- und Adress-Pins).

---
