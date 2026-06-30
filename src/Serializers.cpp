#include "App.h"

void serializeCategory(JsonObject obj, const Category& category) {
  obj["id"] = category.id;
  obj["name"] = category.name;
  obj["active"] = category.active;
}

void serializeProduct(JsonObject obj, const Product& product) {
  Category* category = findCategoryById(product.categoryId);

  obj["id"] = product.id;
  obj["name"] = product.name;
  obj["price"] = product.price;
  obj["active"] = product.active;
  obj["categoryId"] = product.categoryId;
  obj["category"] = category != nullptr ? category->name : product.category;
  obj["description"] = product.description;
  obj["image"] = product.image;
}

void serializeExtra(JsonObject obj, const Extra& extra) {
  obj["id"] = extra.id;
  obj["productId"] = extra.productId;
  obj["name"] = extra.name;
  obj["price"] = extra.price;
  obj["active"] = extra.active;
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
  obj["notes"] = order.notes;

  JsonArray items = obj["items"].to<JsonArray>();

  for (int i = 0; i < order.itemCount; i++) {
    JsonObject item = items.add<JsonObject>();
    item["productId"] = order.items[i].productId;
    item["name"] = order.items[i].name;
    item["qty"] = order.items[i].qty;
    item["price"] = order.items[i].price;
    item["image"] = order.items[i].image;
    item["notes"] = order.items[i].notes;

    JsonArray itemExtras = item["extras"].to<JsonArray>();
    for (int j = 0; j < order.items[i].extraCount; j++) {
      JsonObject extra = itemExtras.add<JsonObject>();
      serializeExtra(extra, order.items[i].extras[j]);
    }
  }
}

String buildCategoriesJson(bool onlyActive) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < categoryCount; i++) {
    if (onlyActive && !categories[i].active) continue;
    JsonObject obj = arr.add<JsonObject>();
    serializeCategory(obj, categories[i]);
  }

  String response;
  serializeJson(doc, response);
  return response;
}

String buildProductsJson(bool onlyActive) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < productCount; i++) {
    Category* category = findCategoryById(products[i].categoryId);
    if (onlyActive && (!products[i].active || category == nullptr || !category->active)) continue;
    JsonObject obj = arr.add<JsonObject>();
    serializeProduct(obj, products[i]);
  }

  String response;
  serializeJson(doc, response);
  return response;
}

String buildExtrasJson(bool onlyActive) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < extraCount; i++) {
    if (onlyActive && !extras[i].active) continue;
    JsonObject obj = arr.add<JsonObject>();
    serializeExtra(obj, extras[i]);
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
  doc["apSsidEffective"] = buildEffectiveApSsid();
  doc["moduleId"] = getModuleId();
  doc["apPassword"] = settings.apPassword;
  doc["counterModeEnabled"] = settings.counterModeEnabled;
  doc["ip"] = WiFi.softAPIP().toString();
  doc["productCount"] = productCount;
  doc["categoryCount"] = categoryCount;
  doc["extraCount"] = extraCount;
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
  doc["apSsidEffective"] = buildEffectiveApSsid();
  doc["moduleId"] = getModuleId();

  String response;
  serializeJson(doc, response);
  return response;
}
