#include "MqttPublisher.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"

static WiFiClientSecure wifiClient;
static PubSubClient mqttClient(wifiClient);

void MqttPublisher::begin() {
  // HiveMQ Cloud exige TLS. Para simplificar o TCC, pulamos a validacao
  // do certificado (setInsecure). Em producao real, o ideal seria usar
  // wifiClient.setCACert(...) com o certificado raiz correto.
  wifiClient.setInsecure();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void MqttPublisher::reconnectIfNeeded() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao broker MQTT... Broker: ");
    Serial.print(MQTT_BROKER);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT conectado!");
    } else {
      Serial.print("Falha ao conectar, rc=");
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

  struct tm timeinfo;
  char horaFormatada[25];
  if (getLocalTime(&timeinfo)) {
    strftime(horaFormatada, sizeof(horaFormatada), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(horaFormatada, "hora_indisponivel");
  }

  char tempStr[10];
  char umidStr[10];
  dtostrf(reading.temperature, 0, 3, tempStr);
  dtostrf(reading.humidity, 0, 3, umidStr);

  StaticJsonDocument<200> doc;
  doc["id"] = MQTT_CLIENT_ID;
  doc["hora"] = horaFormatada;
  doc["temperatura"] = serialized(tempStr);
  doc["umidade"] = serialized(umidStr);

  char payload[200];
  size_t len = serializeJson(doc, payload);

  if (mqttClient.publish(MQTT_TOPIC, payload, len)) {
    Serial.println("Publicado: " + String(payload));
  } else {
    Serial.println("Falha ao publicar no MQTT.");
  }
}
