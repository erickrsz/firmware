#include "MeshManager.h"
#include <painlessMesh.h>
#include "config.h"

static painlessMesh mesh;
static Scheduler userScheduler;
static std::function<void(const String&)> onMeshMessageCallback = nullptr;

static void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Mensagem recebida via mesh de %u: %s\n", from, msg.c_str());
  if (IS_ROOT_NODE && onMeshMessageCallback) {
    onMeshMessageCallback(msg);
  }
}

static void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("Novo no conectado ao mesh: %u\n", nodeId);
}

static void changedConnectionCallback() {
  Serial.printf("Topologia do mesh mudou. Total de nos: %d\n", mesh.getNodeList().size());
}

void MeshManager::begin() {
  mesh.setDebugMsgTypes(ERROR | STARTUP);

  // IMPORTANTE: o canal precisa bater com o canal do seu roteador
  // Wi-Fi (definido em MESH_CHANNEL no config.h). O ESP32 so usa um
  // canal de radio por vez para mesh + Wi-Fi simultaneos - se o
  // roteador estiver em canal diferente, o Wi-Fi nunca conecta
  // (mesmo com SSID/senha corretos), travando com "Falha ao
  // conectar ao Wi-Fi via mesh bridge".
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  if (IS_ROOT_NODE) {
    mesh.stationManual(WIFI_SSID, WIFI_PASSWORD);
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    Serial.println("Este no esta configurado como RAIZ (ponte com Wi-Fi/MQTT).");
  } else {
    Serial.println("Este no esta configurado como SENSOR (apenas mesh).");
  }
}

void MeshManager::update() {
  mesh.update();
}

void MeshManager::broadcastReading(const String& jsonPayload) {
  mesh.sendBroadcast(jsonPayload);
}

void MeshManager::setOnMeshMessage(std::function<void(const String&)> callback) {
  onMeshMessageCallback = callback;
}
