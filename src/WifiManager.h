#pragma once

class WifiManager {
public:
  // So sincroniza a hora via NTP. So deve ser chamada depois que o
  // Wi-Fi ja estiver de fato conectado (WiFi.status() == WL_CONNECTED).
  static void syncNtp();
  static bool isConnected();
};
