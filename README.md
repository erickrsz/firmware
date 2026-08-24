# Firmware — Calculadora Agrícola IoT (nó sensor)

## Arquitetura

```
main.cpp
  ├── WifiManager     → conecta e mantém a conexão Wi-Fi
  ├── DhtSensor        → lê e valida temperatura/umidade do DHT22
  └── MqttPublisher     → monta o payload JSON e publica no broker MQTT

config.h  → configurações compartilhadas por todos os módulos
```

## Fluxo de execução

1. `setup()`: inicializa o sensor, conecta ao Wi-Fi e configura o cliente MQTT.
2. `loop()`: garante que Wi-Fi e MQTT estejam conectados, e a cada
   `READ_INTERVAL_MS` (30 minutos por padrão) lê o sensor e publica o
   resultado no broker.

## Payload publicado (tópico `MQTT_TOPIC`)

```json
{
  "id": "esp32_no_sensor_01",
  "timestamp": 123456,
  "temperatura": 24.5,
  "umidade": 65.2
}
```

## Como usar

### Opção 1 — PlatformIO (recomendado)

1. Abra a pasta `firmware/` no VS Code com a extensão PlatformIO.
2. Edite `src/config.h` com seu SSID, senha de Wi-Fi e endereço do broker MQTT.
3. Conecte o ESP32 via USB e clique em "Upload".

### Opção 2 — Arduino IDE

1. Copie todos os arquivos de `src/` para uma pasta com o mesmo nome do
   arquivo principal (ex: `main/main.ino`, `main/config.h`, etc. — a
   Arduino IDE exige que o `.ino` tenha o mesmo nome da pasta).
2. Instale as bibliotecas listadas em `platformio.ini` pelo Gerenciador
   de Bibliotecas: **DHT sensor library** (Adafruit), **Adafruit Unified
   Sensor**, **PubSubClient**, **ArduinoJson**.
3. Edite `config.h` com suas credenciais.
4. Selecione a placa "ESP32 Dev Module" e faça o upload.

## Próximos passos sugeridos

- Adicionar modo *deep sleep* entre leituras para economizar bateria em campo.
- Adicionar retry/backoff exponencial na reconexão Wi-Fi/MQTT.
- Persistir leituras localmente (ex: LittleFS) caso o Wi-Fi caia, para
  reenviar quando a conexão voltar.
