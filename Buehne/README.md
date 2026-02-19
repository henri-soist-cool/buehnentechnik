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
