#include "WifiManager.h"
#include <Arduino.h>
#include <WiFi.h>
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
  } else {
    Serial.println("\nFalha ao conectar ao Wi-Fi.");
  }
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
