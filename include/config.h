#pragma once

// ---------- Wi-Fi ----------
#define WIFI_SSID     "Batata Bolota"
#define WIFI_PASSWORD "aneerick1234"

// ---------- MQTT ----------
#define MQTT_BROKER    "test.mosquitto.org"
#define MQTT_PORT      1883
#define MQTT_TOPIC     "calculadora_agricola/leituras"
#define MQTT_CLIENT_ID "esp32_no_sensor_01"

// ---------- Sensor ----------
#define DHTPIN  4
#define DHTTYPE DHT22

// ---------- Timing ----------
// 30 minutos, conforme metodologia do TCC (intervalo fixo de amostragem)
#define READ_INTERVAL_MS 5000UL // 5 segundos, só para ver o loop rodando

// ---------- NTP (hora real) ----------
#define NTP_SERVER     "pool.ntp.org"
#define GMT_OFFSET_SEC -10800   // UTC-3 (horário de Brasília, sem horário de verão)
#define DAYLIGHT_OFFSET_SEC 0