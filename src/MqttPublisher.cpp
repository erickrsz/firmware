#include "MqttPublisher.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
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

  // Hora real via NTP (sincronizada no WifiManager::connect())
  struct tm timeinfo;
  char horaFormatada[25];
  if (getLocalTime(&timeinfo)) {
    strftime(horaFormatada, sizeof(horaFormatada), "%Y-%m-%d %H:%M:%S", &timeinfo);
  } else {
    strcpy(horaFormatada, "hora_indisponivel");
  }

  // Formata temperatura e umidade com exatamente 3 casas decimais,
  // evitando o "ruido" de precisao do float (ex: 19.10000038 -> 19.100)
  char tempStr[10];
  char umidStr[10];
  dtostrf(reading.temperature, 0, 3, tempStr);
  dtostrf(reading.humidity, 0, 3, umidStr);

  // Payload conforme fluxograma do TCC: hora, T, UR, id
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