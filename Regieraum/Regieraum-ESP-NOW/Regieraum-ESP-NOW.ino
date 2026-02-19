#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <esp_now.h>
#include <WiFi.h>

// --- Display ---
MatrixPanel_I2S_DMA *display = nullptr;
HUB75_I2S_CFG mxconfig(64, 32, 1); // 64 breit, 32 hoch, 1 Panel, PIN-mapping

// --- Timeout / Idle ---
unsigned long lastUpdate = 0;
const unsigned long timeoutMs = 30000; // 30 Sek
bool inIdle = true;   // Idle direkt aktiv, Display zeigt bereit

// --- Datenstruktur für ESP-NOW ---
typedef struct {
  int number;
} esp_now_message_t;

// --- Farbzuordnung ---
uint16_t getColorForNumber(int n) {
  switch (n) {
    case 1: return 0xFFFF; // Weiß - Spot an
    case 2: return 0xF800; // Rot - Licht an
    case 3: return 0x001F; // Blau - Vorhang auf
    case 4: return 0x07E0; // Grün - Mirko an
    case 5: return 0xFFE0; // Gelb - Umbauf fertig
  }
  return 0;
}
// --- Textzuordnung ---
char getTextForNumber(int n) 
{
  switch (n) {
    case 1: return "Spot an"; // Weiß - Spot an
    case 2: return "Licht an"; // Rot - Licht an
    case 3: return "Vorhang auf"; // Blau - Vorhang auf
    case 4: return "Mikro an"; // Grün - Mirko an
    case 5: return "Umbau fertig"; // Gelb - Umbauf fertig
  }
  return "Bereit";
}

// --- Idle-Screen ---
void showIdleScreen() {
  display->fillScreen(0);
  display->setBrightness8(50);

  display->setTextColor(display->color565(0, 0, 255));  // Blau
  display->setTextSize(1);
  display->setCursor(2, 24);   // Unten platzieren
  display->print("Bereit");
}

// --- Zahl im Kreis ---
void drawNumberCircle(int n) {
  display->fillScreen(0);

  uint16_t color = getColorForNumber(n);

  // Kreis
  int cx = 32;
  int cy = 16;
  int r = 14;
  display->drawCircle(cx, cy, r, color);

  // Zahl
  display->setTextColor(color);
  display->setTextSize(2);

  int x = 28;
  int y = 10;
  display->setCursor(x, y);
  display->print(n);
}

// --- Callback: ESP-NOW Daten empfangen ---
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  esp_now_message_t msg;
  memcpy(&msg, incomingData, sizeof(msg));

  lastUpdate = millis();
  inIdle = false;

  display->setBrightness8(128);
  drawNumberCircle(msg.number);
}

void setup() {
  Serial.begin(115200);

  // --- HUB75 CONFIG ---
  mxconfig.driver = HUB75_I2S_CFG::FM6124;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;

  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();
  display->setFont();

  // Idle direkt anzeigen
  showIdleScreen();

  // --- ESP-NOW Setup ---
  WiFi.mode(WIFI_STA);  // ESP-NOW benötigt STA oder AP-STA
  WiFi.disconnect();

  // MAC-Adresse anzeigen
  Serial.print("Meine MAC-Adresse: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init fehlgeschlagen!");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.println("ESP-NOW bereit.");
}


void loop() {
  // --- Timeout -> Idle ---
  if (!inIdle && millis() - lastUpdate > timeoutMs) {
    inIdle = true;
    showIdleScreen();
  }
}
