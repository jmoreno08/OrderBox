#include "App.h"

String statusToString(OrderStatus status) {
  switch (status) {
    case RECEIVED: return "RECEIVED";
    case PREPARING: return "PREPARING";
    case READY: return "READY";
    case DELIVERED: return "DELIVERED";
    case CANCELLED: return "CANCELLED";
    default: return "UNKNOWN";
  }
}

OrderStatus stringToStatus(const String& status) {
  if (status == "PREPARING") return PREPARING;
  if (status == "READY") return READY;
  if (status == "DELIVERED") return DELIVERED;
  if (status == "CANCELLED") return CANCELLED;
  return RECEIVED;
}

String readBody(AsyncWebServerRequest *request) {
  if (request->hasParam("body", true)) {
    return request->getParam("body", true)->value();
  }
  return "";
}

String getModuleId() {
  uint64_t mac = ESP.getEfuseMac();
  char id[7];
  snprintf(id, sizeof(id), "%06X", (uint32_t)(mac & 0xFFFFFF));
  return String(id);
}

String buildEffectiveApSsid() {
  String base = settings.apSsid.length() ? settings.apSsid : WIFI_AP_SSID_DEFAULT;
  String suffix = "-" + getModuleId();
  int maxBaseLength = 32 - suffix.length();

  if (maxBaseLength < 1) maxBaseLength = 1;
  if (base.length() > (size_t)maxBaseLength) {
    base = base.substring(0, maxBaseLength);
  }

  return base + suffix;
}

Product* findProductById(int id) {
  for (int i = 0; i < productCount; i++) {
    if (products[i].id == id) return &products[i];
  }
  return nullptr;
}

Category* findCategoryById(int id) {
  for (int i = 0; i < categoryCount; i++) {
    if (categories[i].id == id) return &categories[i];
  }
  return nullptr;
}

Extra* findExtraById(int id) {
  for (int i = 0; i < extraCount; i++) {
    if (extras[i].id == id) return &extras[i];
  }
  return nullptr;
}

TableInfo* findTableById(const String& id) {
  for (int i = 0; i < tableCount; i++) {
    if (tables[i].id == id) return &tables[i];
  }
  return nullptr;
}

Order* findOrderById(int id) {
  for (int i = 0; i < orderCount; i++) {
    if (orders[i].id == id) return &orders[i];
  }
  return nullptr;
}

void sendJsonError(AsyncWebServerRequest *request, int code, const String& message) {
  JsonDocument doc;
  doc["error"] = message;
  String response;
  serializeJson(doc, response);
  request->send(code, "application/json", response);
}

