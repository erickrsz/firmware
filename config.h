#pragma once

// ---------- Wi-Fi ----------
#define WIFI_SSID     "WIFI"
#define WIFI_PASSWORD "SENHA"

// ---------- MQTT ----------
#define MQTT_BROKER    "broker.exemplo.com"
#define MQTT_PORT      1883
#define MQTT_TOPIC     "calculadora_agricola/leituras"
#define MQTT_CLIENT_ID "esp32_no_sensor_01"

// ---------- Sensor ----------
#define DHTPIN  4
#define DHTTYPE DHT22

// ---------- Timing ----------
// 30 minutos, conforme metodologia do TCC (intervalo fixo de amostragem)
#define READ_INTERVAL_MS 1800000UL
