#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "WifiManager.h"
#include "DhtSensor.h"
#include "MqttPublisher.h"
#include "MeshManager.h"

unsigned long lastReadTime = 0;

// So chamado no no raiz (ver MeshManager::setOnMeshMessage em setup()).
void onMeshMessageReceived(const String& payload) {
  MqttPublisher::forwardMeshPayload(payload);
}

void setup() {
  Serial.begin(115200);
  DhtSensor::begin();

  MeshManager::begin();

  if (IS_ROOT_NODE) {
    MeshManager::setOnMeshMessage(onMeshMessageReceived);
    WifiManager::waitForConnectionAndSyncNtp();
    MqttPublisher::begin();
  }
}

void loop() {
  // mesh.update() precisa rodar sempre, em todo ciclo do loop,
  // independente do papel deste no.
  MeshManager::update();

  if (IS_ROOT_NODE) {
    MqttPublisher::reconnectIfNeeded();
  }

  if (lastReadTime == 0 || millis() - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = millis();

    SensorReading reading = DhtSensor::read();

    if (!reading.valid) {
      Serial.println("Leitura invalida, nao enviado.");
      return;
    }

    if (IS_ROOT_NODE) {
      // O no raiz publica direto no MQTT, com sua propria hora real.
      MqttPublisher::publishReading(reading, NODE_ID);
    } else {
      // Nos-sensores nao tem Wi-Fi/NTP proprio - mandam so id + valores
      // pela mesh; o no raiz completa a hora ao repassar ao MQTT.
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
