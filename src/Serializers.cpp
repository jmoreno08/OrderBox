#include "App.h"

void serializeProduct(JsonObject obj, const Product& product) {
  obj["id"] = product.id;
  obj["name"] = product.name;
  obj["price"] = product.price;
  obj["active"] = product.active;
  obj["category"] = product.category;
  obj["description"] = product.description;
  obj["image"] = product.image;
}

void serializeTable(JsonObject obj, const TableInfo& table) {
  obj["id"] = table.id;
  obj["name"] = table.name;
  obj["locked"] = table.locked;
}

void serializeOrder(JsonObject obj, const Order& order) {
  obj["id"] = order.id;
  obj["sourceType"] = order.sourceType;
  obj["table"] = order.table;
  obj["counterNumber"] = order.counterNumber;
  obj["status"] = statusToString(order.status);
  obj["createdAt"] = order.createdAt;

  JsonArray items = obj["items"].to<JsonArray>();

  for (int i = 0; i < order.itemCount; i++) {
    JsonObject item = items.add<JsonObject>();
    item["productId"] = order.items[i].productId;
    item["name"] = order.items[i].name;
    item["qty"] = order.items[i].qty;
    item["price"] = order.items[i].price;
    item["image"] = order.items[i].image;
  }
}

String buildProductsJson(bool onlyActive) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < productCount; i++) {
    if (onlyActive && !products[i].active) continue;
    JsonObject obj = arr.add<JsonObject>();
    serializeProduct(obj, products[i]);
  }

  String response;
  serializeJson(doc, response);
  return response;
}

String buildTablesJson() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < tableCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    serializeTable(obj, tables[i]);
  }

  String response;
  serializeJson(doc, response);
  return response;
}

String buildOrdersJson() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < orderCount; i++) {
    JsonObject obj = arr.add<JsonObject>();
    serializeOrder(obj, orders[i]);
  }

  String response;
  serializeJson(doc, response);
  return response;
}

String buildSettingsJson() {
  JsonDocument doc;
  doc["businessName"] = settings.businessName;
  doc["apSsid"] = settings.apSsid;
  doc["apPassword"] = settings.apPassword;
  doc["counterModeEnabled"] = settings.counterModeEnabled;
  doc["ip"] = WiFi.softAPIP().toString();
  doc["productCount"] = productCount;
  doc["tableCount"] = tableCount;
  doc["orderCount"] = orderCount;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["chipModel"] = ESP.getChipModel();

  String response;
  serializeJson(doc, response);
  return response;
}

String buildPublicSettingsJson() {
  JsonDocument doc;
  doc["businessName"] = settings.businessName;
  doc["counterModeEnabled"] = settings.counterModeEnabled;

  String response;
  serializeJson(doc, response);
  return response;
}

// ===============================
