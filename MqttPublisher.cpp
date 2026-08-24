#include "MqttPublisher.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

void MqttPublisher::begin() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void MqttPublisher::reconnectIfNeeded() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println(" conectado.");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
  mqttClient.loop();
}

void MqttPublisher::publishReading(const SensorReading& reading) {
  if (!reading.valid) {
    Serial.println("Leitura invalida, nao publicado.");
    return;
  }

  // Payload conforme fluxograma do TCC: timestamp, T, UR, id
  StaticJsonDocument<200> doc;
  doc["id"] = MQTT_CLIENT_ID;
  doc["timestamp"] = millis();
  doc["temperatura"] = reading.temperature;
  doc["umidade"] = reading.humidity;

  char payload[200];
  size_t len = serializeJson(doc, payload);

  if (mqttClient.publish(MQTT_TOPIC, payload, len)) {
    Serial.println("Publicado: " + String(payload));
  } else {
    Serial.println("Falha ao publicar no MQTT.");
  }
}
