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
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  if (IS_ROOT_NODE) {
    // O no raiz mantem uma perna no mesh e outra no Wi-Fi normal,
    // servindo de ponte entre os dois. E o painlessMesh, nao o
    // WifiManager, quem inicia essa conexao Wi-Fi neste caso.
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
