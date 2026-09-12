#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "WifiManager.h"
#include "DhtSensor.h"
#include "MqttPublisher.h"
#include "MeshManager.h"

unsigned long lastReadTime = 0;

void onMeshMessageReceived(const String& payload) {
  MqttPublisher::forwardMeshPayload(payload);
}

void setup() {
  Serial.begin(115200);
  DhtSensor::begin();

  MeshManager::begin();

    if (IS_ROOT_NODE) {
      MeshManager::setOnMeshMessage(onMeshMessageReceived);

      // IMPORTANTE: mesh.stationManual() (chamado dentro de
      // MeshManager::begin()) so AGENDA a tentativa de conexao - quem
      // processa essa tentativa de verdade e o scheduler interno do
      // painlessMesh, que so roda quando chamamos MeshManager::update()
      // (mesh.update()) repetidamente. Por isso, aqui usamos um loop
      // que continua chamando update() enquanto espera, em vez de um
      // delay() bloqueante que travaria a mesh inteira.
      Serial.print("Aguardando Wi-Fi (via mesh bridge)");
      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        MeshManager::update();
        delay(50);
        if ((millis() - start) % 1000 < 50) Serial.print(".");
      }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());
      WifiManager::syncNtp();
    } else {
      Serial.println("\nFalha ao conectar ao Wi-Fi via mesh bridge (20s).");
    }

    MqttPublisher::begin();
  }
}

void loop() {
  MeshManager::update();

  if (IS_ROOT_NODE) {
    if (WiFi.status() == WL_CONNECTED) {
      MqttPublisher::reconnectIfNeeded();
    }
  }

  if (lastReadTime == 0 || millis() - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = millis();

    SensorReading reading = DhtSensor::read();

    if (!reading.valid) {
      Serial.println("Leitura invalida, nao enviado.");
      return;
    }

    if (IS_ROOT_NODE) {
      if (WiFi.status() == WL_CONNECTED) {
        MqttPublisher::publishReading(reading, NODE_ID);
      } else {
        Serial.println("Wi-Fi indisponivel, leitura descartada (root sem conexao).");
      }
    } else {
      StaticJsonDocument<128> doc;
      doc["id"] = NODE_ID;

      char tempStr[10];
      char umidStr[10];
      dtostrf(reading.temperature, 0, 3, tempStr);
      dtostrf(reading.humidity, 0, 3, umidStr);
      doc["temperatura"] = serialized(tempStr);
      doc["umidade"] = serialized(umidStr);

      String payload;
      serializeJson(doc, payload);

      MeshManager::broadcastReading(payload);
      Serial.println("Enviado via mesh: " + payload);
    }
  }
}
