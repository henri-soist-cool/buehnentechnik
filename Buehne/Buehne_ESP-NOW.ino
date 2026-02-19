#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// --- ESP‑NOW Nachricht ---
typedef struct {
  int button;
} ButtonMessage;

// --- Button-Pins ---
// 1: Weiß an GPIO 13
// 2: Rot an GPIO 12
// 3: Blau an GPIO 14
// 4: Grün an GPIO 27
// 5: Gelb an GPIO 26
const int buttonPins[5] = { 13, 12, 14, 27, 26 };

// --- Onboard-LED (ESP32 DevKit: GPIO 2) ---
const int ledPin = 2;

// Die Buttons haben bei Druck merkwürdig reagiert. Software-Debounce: 
// https://www.digikey.de/de/maker/tutorials/2024/how-to-implement-a-software-based-debounce-algorithm-for-button-inputs-on-a-microcontroller
// --- Entprellung ---
const uint32_t debounceTimeMs = 40;   // Entprellzeit

// Zustände und Zeiten pro Button
bool lastStableState[5];           // letzter stabiler Zustand
bool lastReadState[5];             // letzter gelesener Zustand
uint32_t lastChangeTime[5];        // Zeitpunkt der letzten Zustandsänderung

// MAC-Adresse des Display-ESP (Empfänger)
uint8_t receiverMac[] = {0x44, 0x1D, 0x64, 0xF7, 0x36, 0x24};

// Forward
void buttonTask(void* parameter);

// --- Callback: Sende-Status ---
void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Sendestatus: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "Fehler");
}


void setup() {
  Serial.begin(115200);
  Serial.println("ESP1 Sender startet...");

  // Buttons als INPUT_PULLUP
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    bool initialState = digitalRead(buttonPins[i]);  // HIGH im Ruhezustand
    lastStableState[i] = initialState;
    lastReadState[i] = initialState;
    lastChangeTime[i] = millis();
  }

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // --- ESP‑NOW starten ---
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen!");
    return;
  }

  esp_now_register_send_cb(onSent);

  // Verbindung zum Display-ESP herstellen (Peer hinzufügen)
  Serial.println("Verbinde zu ESP_DISPLAY...");

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("ESP-NOW Peer hinzugefügt!");
  } else {
      Serial.println("Peer hinzufügen fehlgeschlagen!");
  }
  
  // Task für Button-Handling
  xTaskCreate(
    buttonTask,
    "ButtonTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop() {
  // Nichts zu tun, alles läuft in der Task
  vTaskDelay(portMAX_DELAY);
}

// --- Task: Buttons pollen + Software-Entprellung ---
void buttonTask(void* parameter) {
  const TickType_t pollDelay = pdMS_TO_TICKS(5); // alle 5 ms prüfen

  while (true) {

    uint32_t now = millis();

    for (int i = 0; i < 5; i++) {
      bool currentState = digitalRead(buttonPins[i]); // HIGH = nicht gedrückt, LOW = gedrückt

      if (currentState != lastReadState[i]) {
        // Zustand hat sich geändert -> Zeit merken
        lastReadState[i] = currentState;
        lastChangeTime[i] = now;
      }

      // Prüfen, ob der neue Zustand lange genug stabil ist
      if ((now - lastChangeTime[i]) >= debounceTimeMs) {
        // Wenn sich der stabile Zustand ändert, ist das ein gültiges Event
        if (currentState != lastStableState[i]) {
          lastStableState[i] = currentState;

          // Wir reagieren nur auf "gedrückt" (LOW bei INPUT_PULLUP)
          if (currentState == LOW) {
            int buttonNumber = i + 1;

            Serial.printf("Button %d sauber gedrueckt\n", buttonNumber);

            // ESP‑NOW: Zahl als Struktur senden
            ButtonMessage msg;
            msg.button = buttonNumber;
            esp_now_send(receiverMac, (uint8_t*)&msg, sizeof(msg));

            // Kurze LED-Quittung
            digitalWrite(ledPin, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(ledPin, LOW);
          }
        }
      }
    }

    vTaskDelay(pollDelay);
  }
}
