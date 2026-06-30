#include <Arduino.h>
#include "src/App.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS error");
    return;
  }

  loadAllData();
  setupWiFiAP();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  setupRoutes();
  server.begin();

  Serial.println("Servidor iniciado");
}

void loop() {
  ws.cleanupClients();
}
