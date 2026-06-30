#include "App.h"

void broadcastDoc(JsonDocument& doc) {
  String message;
  serializeJson(doc, message);
  ws.textAll(message);
}

void broadcastOrderEvent(const String& eventName, const Order& order) {
  JsonDocument doc;
  doc["event"] = eventName;
  JsonObject data = doc["data"].to<JsonObject>();
  serializeOrder(data, order);
  broadcastDoc(doc);
}

void broadcastProductEvent(const String& eventName, const Product& product) {
  JsonDocument doc;
  doc["event"] = eventName;
  JsonObject data = doc["data"].to<JsonObject>();
  serializeProduct(data, product);
  broadcastDoc(doc);
}

void broadcastTableEvent(const String& eventName, const TableInfo& table) {
  JsonDocument doc;
  doc["event"] = eventName;
  JsonObject data = doc["data"].to<JsonObject>();
  serializeTable(data, table);
  broadcastDoc(doc);
}

void broadcastSettingsEvent() {
  JsonDocument doc;
  doc["event"] = "settingsUpdated";
  JsonObject data = doc["data"].to<JsonObject>();
  data["businessName"] = settings.businessName;
  data["apSsid"] = settings.apSsid;
  data["counterModeEnabled"] = settings.counterModeEnabled;
  broadcastDoc(doc);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (!(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)) return;

  data[len] = 0;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (char*)data);

  if (error) return;

  String action = doc["action"] | "";

  if (action == "ping") {
    JsonDocument response;
    response["event"] = "heartbeat";
    broadcastDoc(response);
  }
}

void onWsEvent(
  AsyncWebSocket *server,
  AsyncWebSocketClient *client,
  AwsEventType type,
  void *arg,
  uint8_t *data,
  size_t len
) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("WebSocket conectado: #%u\n", client->id());
    client->text("{\"event\":\"connected\"}");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("WebSocket desconectado: #%u\n", client->id());
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

// ===============================
// WiFi
// ===============================
