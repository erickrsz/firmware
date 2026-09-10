#pragma once
#include <Arduino.h>
#include <functional>

class MeshManager {
public:
  static void begin();
  static void update();
  static void broadcastReading(const String& jsonPayload);

  // So tem efeito no no raiz - registra quem trata mensagens
  // recebidas de outros nos da mesh, para repassar ao MQTT.
  static void setOnMeshMessage(std::function<void(const String&)> callback);
};
