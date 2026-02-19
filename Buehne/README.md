# 📡 ESP‑NOW Button‑Sender – Dokumentation

Dieses Projekt implementiert einen **ESP32‑basierten Funksender**, der fünf Taster überwacht und deren Status per **ESP‑NOW** an einen Empfänger (z. B. ein Display‑ESP32) sendet.  
Die Buttons sind **entprellt**, laufen in einer **FreeRTOS‑Task** und senden eine **strukturierte Nachricht**.

---

## 🧩 Projektüberblick

Der ESP32:

- überwacht **5 Buttons** (INPUT_PULLUP)
- entprellt sie per Software
- sendet bei jedem gültigen Tastendruck eine **ESP‑NOW‑Nachricht**
- quittiert den Tastendruck mit der **Onboard‑LED**
- läuft vollständig **asynchron** in einer FreeRTOS‑Task

---

## 📦 Datenstruktur

Die Nachricht besteht aus einer einfachen Struktur:

```cpp
typedef struct {
  int button;
} ButtonMessage;
```

Der Empfänger erhält also nur die Nummer des gedrückten Buttons.

---

## 🔌 Hardware‑Belegung

| Button | Farbe | GPIO |
|--------|--------|-------|
| 1 | Weiß | 13 |
| 2 | Rot | 12 |
| 3 | Blau | 14 |
| 4 | Grün | 27 |
| 5 | Gelb | 26 |

Alle Buttons sind als **INPUT_PULLUP** konfiguriert →  
**LOW = gedrückt**, **HIGH = nicht gedrückt**.

Die Onboard‑LED (GPIO 2) dient als visuelle Rückmeldung.

---

## 🛠️ Initialisierung

### Buttons vorbereiten

Beim Start werden alle Pins als `INPUT_PULLUP` gesetzt und die Entprell‑Variablen initialisiert:

```cpp
pinMode(buttonPins[i], INPUT_PULLUP);
lastStableState[i] = digitalRead(buttonPins[i]);
```

---

## 📡 ESP‑NOW Setup

Der ESP32 wird in den **STA‑Modus** gesetzt und ESP‑NOW gestartet:

```cpp
WiFi.mode(WIFI_STA);
esp_now_init();
```

Anschließend wird der Empfänger als Peer eingetragen:

```cpp
esp_now_peer_info_t peerInfo = {};
memcpy(peerInfo.peer_addr, receiverMac, 6);
esp_now_add_peer(&peerInfo);
```

Die MAC‑Adresse des Empfängers ist fest im Code hinterlegt.

---

## 🚀 Senden einer Nachricht

Beim Tastendruck wird eine Struktur gesendet:

```cpp
ButtonMessage msg;
msg.button = buttonNumber;
esp_now_send(receiverMac, (uint8_t*)&msg, sizeof(msg));
```

Der Sendestatus wird über einen Callback ausgegeben:

```cpp
void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fehler");
}
```

---

## 🧠 Software‑Entprellung

Die Entprellung basiert auf einem stabilen Zeitfenster:

- Jeder Button wird alle **5 ms** abgefragt
- Wenn ein neuer Zustand **mindestens 40 ms stabil** bleibt, gilt er als gültig
- Nur der Übergang **HIGH → LOW** löst ein Event aus

Ablauf:

1. Zustand lesen  
2. Wenn anders als vorher → Zeit merken  
3. Wenn stabil länger als `debounceTimeMs` → gültig  
4. Wenn gedrückt → Nachricht senden

---

## 🧵 FreeRTOS‑Task

Die gesamte Button‑Logik läuft in einer eigenen Task:

```cpp
xTaskCreate(buttonTask, "ButtonTask", 4096, NULL, 1, NULL);
```

Die Task pollt die Buttons:

```cpp
const TickType_t pollDelay = pdMS_TO_TICKS(5);
vTaskDelay(pollDelay);
```

Der `loop()` bleibt leer:

```cpp
void loop() {
  vTaskDelay(portMAX_DELAY);
}
```

---

## 💡 LED‑Feedback

Nach jedem erfolgreichen Tastendruck blinkt die Onboard‑LED kurz:

```cpp
digitalWrite(ledPin, HIGH);
vTaskDelay(pdMS_TO_TICKS(100));
digitalWrite(ledPin, LOW);
```

---

## 📝 Zusammenfassung

Dieses Projekt bietet:

- zuverlässige Button‑Erkennung  
- robuste Software‑Entprellung  
- schnelle, stromsparende ESP‑NOW‑Übertragung  
- klare Trennung von Setup, Loop und Task  
- einfache Erweiterbarkeit (weitere Buttons, andere Payloads)

---
