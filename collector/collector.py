"""
Calculadora Agrícola IoT — Coletor MQTT -> PostgreSQL

Fica permanentemente conectado ao broker MQTT e grava cada leitura
recebida na tabela leitura_iot, resolvendo o problema de perda de
dados que acontece quando o dashboard (navegador) não está aberto.

Uso:
    python collector.py

Credenciais lidas de um arquivo .env (veja .env.example).
"""

import json
import logging
import signal
import sys
import time
from datetime import datetime, date

import psycopg2
import paho.mqtt.client as mqtt
from dotenv import dotenv_values

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
log = logging.getLogger("coletor")

config = dotenv_values(".env")


def env(key, required=True, default=None):
    value = config.get(key, default)
    if required and not value:
        log.error("Variável obrigatória ausente no .env: %s", key)
        sys.exit(1)
    return value


DB_HOST = env("DB_HOST")
DB_PORT = env("DB_PORT", default="5432")
DB_NAME = env("DB_NAME")
DB_USER = env("DB_USER")
DB_PASSWORD = env("DB_PASSWORD")

MQTT_HOST = env("MQTT_HOST")
MQTT_PORT = int(env("MQTT_PORT", default="8883"))
MQTT_USERNAME = env("MQTT_USERNAME")
MQTT_PASSWORD = env("MQTT_PASSWORD")
MQTT_TOPIC = env("MQTT_TOPIC")

# Identificação do sensor/config padrão usados para satisfazer as
# chaves estrangeiras da tabela leitura_iot. Ajuste se seu projeto
# tiver múltiplas regiões/culturas/sensores reais cadastrados.
DEFAULT_REGIAO = ("Regiao de teste", "SP")
DEFAULT_SOLO = ("Nao informado",)
DEFAULT_CULTURA = ("Nao informado", 0)
DEFAULT_SENSOR = ("DHT22 (AM2302)", "Aosong", "MQTT/WiFi", 30, "ativo")


def get_connection():
    return psycopg2.connect(
        host=DB_HOST, port=DB_PORT, dbname=DB_NAME,
        user=DB_USER, password=DB_PASSWORD,
    )


def ensure_defaults(conn):
    """Garante que existe uma configuracao_safra e um sensor_iot
    padrão para referenciar nas leituras, criando-os se necessário
    (idempotente — roda seguro toda vez que o script inicia)."""
    with conn.cursor() as cur:
        cur.execute(
            "SELECT id_regiao FROM regiao WHERE nome_regiao = %s AND uf = %s",
            DEFAULT_REGIAO,
        )
        row = cur.fetchone()
        if row:
            id_regiao = row[0]
        else:
            cur.execute(
                "INSERT INTO regiao (nome_regiao, uf) VALUES (%s, %s) RETURNING id_regiao",
                DEFAULT_REGIAO,
            )
            id_regiao = cur.fetchone()[0]

        cur.execute(
            "SELECT id_solo FROM tipo_solo WHERE descricao_solo = %s", DEFAULT_SOLO
        )
        row = cur.fetchone()
        if row:
            id_solo = row[0]
        else:
            cur.execute(
                "INSERT INTO tipo_solo (descricao_solo) VALUES (%s) RETURNING id_solo",
                DEFAULT_SOLO,
            )
            id_solo = cur.fetchone()[0]

        cur.execute(
            "SELECT id_cultura FROM cultura WHERE nome_cultura = %s",
            (DEFAULT_CULTURA[0],),
        )
        row = cur.fetchone()
        if row:
            id_cultura = row[0]
        else:
            cur.execute(
                "INSERT INTO cultura (nome_cultura, ciclo_estimado_dias) VALUES (%s, %s) RETURNING id_cultura",
                DEFAULT_CULTURA,
            )
            id_cultura = cur.fetchone()[0]

        cur.execute(
            """SELECT id_config FROM configuracao_safra
               WHERE id_regiao = %s AND id_solo = %s AND id_cultura = %s""",
            (id_regiao, id_solo, id_cultura),
        )
        row = cur.fetchone()
        if row:
            id_config = row[0]
        else:
            cur.execute(
                """INSERT INTO configuracao_safra (id_regiao, id_solo, id_cultura, data_plantio)
                   VALUES (%s, %s, %s, %s) RETURNING id_config""",
                (id_regiao, id_solo, id_cultura, date.today()),
            )
            id_config = cur.fetchone()[0]

        cur.execute(
            "SELECT id_sensor FROM sensor_iot WHERE modelo = %s AND fabricante = %s",
            (DEFAULT_SENSOR[0], DEFAULT_SENSOR[1]),
        )
        row = cur.fetchone()
        if row:
            id_sensor = row[0]
        else:
            cur.execute(
                """INSERT INTO sensor_iot
                   (modelo, fabricante, protocolo_comunicacao, intervalo_amostragem_min, status_sensor)
                   VALUES (%s, %s, %s, %s, %s) RETURNING id_sensor""",
                DEFAULT_SENSOR,
            )
            id_sensor = cur.fetchone()[0]

        conn.commit()
        log.info("Defaults prontos: id_config=%s id_sensor=%s", id_config, id_sensor)
        return id_config, id_sensor


def insert_leitura(conn, id_config, id_sensor, temperatura, umidade, hora_str):
    data_hora = None
    if hora_str:
        try:
            data_hora = datetime.strptime(hora_str, "%Y-%m-%d %H:%M:%S")
        except ValueError:
            data_hora = None

    with conn.cursor() as cur:
        if data_hora:
            cur.execute(
                """INSERT INTO leitura_iot (id_config, id_sensor, data_hora, temperatura, umidade_ar)
                   VALUES (%s, %s, %s, %s, %s)""",
                (id_config, id_sensor, data_hora, temperatura, umidade),
            )
        else:
            cur.execute(
                """INSERT INTO leitura_iot (id_config, id_sensor, temperatura, umidade_ar)
                   VALUES (%s, %s, %s, %s)""",
                (id_config, id_sensor, temperatura, umidade),
            )
    conn.commit()


def main():
    conn = get_connection()
    log.info("Conectado ao PostgreSQL (%s/%s)", DB_HOST, DB_NAME)
    id_config, id_sensor = ensure_defaults(conn)

    def on_connect(client, userdata, flags, reason_code, properties=None):
        if reason_code == 0:
            log.info("Conectado ao broker MQTT (%s:%s)", MQTT_HOST, MQTT_PORT)
            client.subscribe(MQTT_TOPIC)
            log.info("Assinando tópico: %s", MQTT_TOPIC)
        else:
            log.error("Falha ao conectar no broker, reason_code=%s", reason_code)

    def on_disconnect(client, userdata, reason_code, properties=None):
        log.warning("Desconectado do broker (reason_code=%s) — reconectando automaticamente...", reason_code)

    def on_message(client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError) as e:
            log.warning("Payload inválido, ignorado: %s", e)
            return

        temperatura = payload.get("temperatura")
        umidade = payload.get("umidade")
        hora = payload.get("hora")

        if temperatura is None or umidade is None:
            log.warning("Payload sem temperatura/umidade, ignorado: %s", payload)
            return

        try:
            insert_leitura(conn, id_config, id_sensor, float(temperatura), float(umidade), hora)
            log.info("Gravado: temperatura=%.3f umidade=%.3f hora=%s", float(temperatura), float(umidade), hora)
        except Exception as e:
            log.error("Erro ao gravar leitura no banco: %s", e)
            conn.rollback()

    client = mqtt.Client(client_id="coletor_calculadora_agricola", protocol=mqtt.MQTTv311)
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.tls_set()  # TLS padrão, exigido pelo HiveMQ Cloud

    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message

    def shutdown(signum, frame):
        log.info("Encerrando coletor...")
        client.disconnect()
        conn.close()
        sys.exit(0)

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    while True:
        try:
            client.connect(MQTT_HOST, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except Exception as e:
            log.error("Erro de conexão MQTT: %s — tentando novamente em 10s", e)
            time.sleep(10)


if __name__ == "__main__":
    main()
