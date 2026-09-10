#pragma once

class WifiManager {
public:
  // So usado pelo no raiz. O MeshManager (via mesh.stationManual)
  // ja inicia a conexao Wi-Fi - esta funcao so espera ela terminar
  // e sincroniza a hora via NTP.
  static void waitForConnectionAndSyncNtp();
  static bool isConnected();
};
