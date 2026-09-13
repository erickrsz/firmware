#include "WifiManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"

void WifiManager::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando ao Wi-Fi");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado. IP: " + WiFi.localIP().toString());

    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

    Serial.print("Sincronizando hora via NTP");
    struct tm timeinfo;
    unsigned long ntpStart = millis();
    while (!getLocalTime(&timeinfo, 1000) && millis() - ntpStart < 10000) {
      Serial.print(".");
    }

    if (getLocalTime(&timeinfo)) {
      char buf[25];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
      Serial.println("\nHora sincronizada: " + String(buf));
    } else {
      Serial.println("\nFalha ao sincronizar hora via NTP.");
    }
  } else {
    Serial.println("\nFalha ao conectar ao Wi-Fi.");
  }
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
