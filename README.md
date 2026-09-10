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

  # Coletor MQTT → PostgreSQL

Resolve o problema de perda de dados quando o dashboard (navegador)
não está aberto: este script fica permanentemente conectado ao
broker MQTT e grava cada leitura recebida no banco de dados,
independente de alguém estar olhando o site ou não.

```
ESP32 → Broker MQTT → collector.py (sempre rodando) → PostgreSQL
                              ↑
                    dashboard passa a consultar
                    o banco em vez do MQTT direto
                    (próximo passo, ainda não implementado)
```

## Pré-requisitos

- Python 3.9+
- Um banco PostgreSQL acessível (local, ou gratuito na nuvem — ver
  seção abaixo)

## Instalação

```powershell
cd collector
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
```

## Configuração

```powershell
Copy-Item .env.example .env
```

Edite o `.env` com os dados reais do seu PostgreSQL e do mesmo
cluster MQTT já usado no firmware/dashboard (atenção: aqui a porta
é **8883**, igual ao firmware — não é a 8884 do dashboard).

## Criar as tabelas no banco

Com o PostgreSQL rodando e o banco já criado (`CREATE DATABASE
calculadora_agricola;`), rode o schema:

```powershell
psql -h localhost -U postgres -d calculadora_agricola -f schema.sql
```

(troque host/usuário conforme o seu `.env`)

## Rodar o coletor

```powershell
python collector.py
```

Deixe esse terminal aberto — é ele que precisa ficar rodando o
tempo todo para não perder leituras. Ele reconecta sozinho se a
internet cair ou o broker ficar indisponível temporariamente.

## Onde rodar isso de verdade (para não depender do seu PC ligado)

Rodar no seu PC já resolve o problema original (não precisa mais do
navegador aberto), mas ainda depende do PC estar ligado. Para
operação 24/7 de verdade, os próximos passos seriam:

- **Banco de dados**: usar um PostgreSQL gratuito na nuvem, como
  [Supabase](https://supabase.com) ou
  [Railway](https://railway.app) (têm planos free adequados ao
  volume de um TCC).
- **O script `collector.py`**: hospedar num serviço que mantém
  processos rodando continuamente, como Railway ou
  [Render](https://render.com) (Background Worker, plano free).

## Verificando se está funcionando

Enquanto o `collector.py` roda, cada leitura recebida aparece no
terminal:

```
2026-08-30 08:15:02 [INFO] Gravado: temperatura=24.500 umidade=65.200 hora=2026-08-30 08:15:00
```

Para conferir direto no banco:

```sql
SELECT * FROM leitura_iot ORDER BY data_hora DESC LIMIT 10;
```

## Próximo passo (ainda pendente)

O dashboard (`index.html` / `medias.html`) ainda escuta o MQTT
diretamente, então ele continua mostrando só o que chega enquanto a
aba está aberta — o `collector.py` grava tudo no banco em paralelo,
mas o site ainda não lê esse banco. Para o dashboard mostrar o
histórico completo (inclusive leituras da madrugada), ele precisaria
de um pequeno backend (ex: uma API Flask/FastAPI) consultando o
PostgreSQL, em vez de assinar o MQTT direto no navegador.
