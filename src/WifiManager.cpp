#include "WifiManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"

void WifiManager::syncNtp() {
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
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
