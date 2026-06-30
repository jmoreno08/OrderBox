#include "App.h"

void handleCreateOrder(AsyncWebServerRequest *request, JsonDocument& doc) {
  if (orderCount >= MAX_ORDERS) {
    sendJsonError(request, 507, "Order storage full");
    return;
  }

  String sourceType = doc["sourceType"] | "table";
  String table = doc["table"] | "";
  JsonArray items = doc["items"].as<JsonArray>();

  if (sourceType != "table" && sourceType != "counter") {
    sendJsonError(request, 400, "Invalid sourceType");
    return;
  }

  if (sourceType == "counter" && !settings.counterModeEnabled) {
    sendJsonError(request, 403, "Counter mode disabled");
    return;
  }

  if (sourceType == "table") {
    if (table.length() == 0) {
      sendJsonError(request, 400, "Table is required");
      return;
    }

    TableInfo* tableInfo = findTableById(table);

    if (tableInfo == nullptr) {
      sendJsonError(request, 404, "Table not found");
      return;
    }

    if (tableInfo->locked) {
      sendJsonError(request, 423, "Table is locked");
      return;
    }
  }

  if (items.size() == 0) {
    sendJsonError(request, 400, "Items are required");
    return;
  }

  Order& order = orders[orderCount];
  order.id = nextOrderId++;
  order.sourceType = sourceType;
  order.table = sourceType == "table" ? table : "";
  order.counterNumber = sourceType == "counter" ? nextCounterNumber++ : 0;
  order.itemCount = 0;
  order.status = RECEIVED;
  order.createdAt = millis();

  for (JsonObject item : items) {
    if (order.itemCount >= MAX_ITEMS_PER_ORDER) break;

    int productId = item["productId"] | 0;
    int qty = item["qty"] | 0;

    Product* product = findProductById(productId);

    if (product != nullptr && product->active && qty > 0) {
      order.items[order.itemCount].productId = product->id;
      order.items[order.itemCount].name = product->name;
      order.items[order.itemCount].qty = qty;
      order.items[order.itemCount].price = product->price;
      order.items[order.itemCount].image = product->image;
      order.itemCount++;
    }
  }

  if (order.itemCount == 0) {
    sendJsonError(request, 400, "No valid active items");
    return;
  }

  orderCount++;
  saveOrders();

  JsonDocument responseDoc;
  JsonObject obj = responseDoc.to<JsonObject>();
  serializeOrder(obj, order);

  String response;
  serializeJson(responseDoc, response);

  request->send(201, "application/json", response);
  broadcastOrderEvent("newOrder", order);
}

void handleOrderUpdate(AsyncWebServerRequest *request, JsonDocument& doc) {
  int orderId = doc["id"] | 0;
  String status = doc["status"] | "RECEIVED";

  Order* order = findOrderById(orderId);

  if (order == nullptr) {
    sendJsonError(request, 404, "Order not found");
    return;
  }

  order->status = stringToStatus(status);
  saveOrders();

  JsonDocument responseDoc;
  JsonObject obj = responseDoc.to<JsonObject>();
  serializeOrder(obj, *order);

  String response;
  serializeJson(responseDoc, response);

  request->send(200, "application/json", response);
  broadcastOrderEvent("orderUpdated", *order);
}

void handleOrderDelete(AsyncWebServerRequest *request, JsonDocument& doc) {
  int orderId = doc["id"] | 0;

  for (int i = 0; i < orderCount; i++) {
    if (orders[i].id == orderId) {
      Order deletedOrder = orders[i];

      for (int j = i; j < orderCount - 1; j++) {
        orders[j] = orders[j + 1];
      }

      orderCount--;
      saveOrders();

      request->send(200, "application/json", "{\"ok\":true}");
      broadcastOrderEvent("orderDeleted", deletedOrder);
      return;
    }
  }

  sendJsonError(request, 404, "Order not found");
}

void handleProductSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  String name = doc["name"] | "";
  float price = doc["price"] | 0;
  bool active = doc["active"] | true;
  String category = doc["category"] | "General";
  String description = doc["description"] | "";
  String image = doc["image"] | "/img/placeholder.svg";

  if (name.length() == 0) {
    sendJsonError(request, 400, "Product name is required");
    return;
  }

  Product* product = nullptr;

  if (id > 0) {
    product = findProductById(id);
  }

  if (product == nullptr) {
    if (productCount >= MAX_PRODUCTS) {
      sendJsonError(request, 507, "Product storage full");
      return;
    }

    Product& created = products[productCount];
    created.id = nextProductId++;
    created.name = name;
    created.price = price;
    created.active = active;
    created.category = category;
    created.description = description;
    created.image = image;
    productCount++;

    saveProducts();

    JsonDocument responseDoc;
    JsonObject obj = responseDoc.to<JsonObject>();
    serializeProduct(obj, created);

    String response;
    serializeJson(responseDoc, response);
    request->send(201, "application/json", response);
    broadcastProductEvent("productSaved", created);
    return;
  }

  product->name = name;
  product->price = price;
  product->active = active;
  product->category = category;
  product->description = description;
  product->image = image;

  saveProducts();

  JsonDocument responseDoc;
  JsonObject obj = responseDoc.to<JsonObject>();
  serializeProduct(obj, *product);

  String response;
  serializeJson(responseDoc, response);
  request->send(200, "application/json", response);
  broadcastProductEvent("productSaved", *product);
}

void handleProductDelete(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;

  for (int i = 0; i < productCount; i++) {
    if (products[i].id == id) {
      Product deletedProduct = products[i];

      for (int j = i; j < productCount - 1; j++) {
        products[j] = products[j + 1];
      }

      productCount--;
      saveProducts();

      request->send(200, "application/json", "{\"ok\":true}");
      broadcastProductEvent("productDeleted", deletedProduct);
      return;
    }
  }

  sendJsonError(request, 404, "Product not found");
}

void handleTableSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  String id = doc["id"] | "";
  String name = doc["name"] | "";
  bool locked = doc["locked"] | false;

  if (id.length() == 0) {
    sendJsonError(request, 400, "Table id is required");
    return;
  }

  if (name.length() == 0) {
    name = "Mesa " + id;
  }

  TableInfo* table = findTableById(id);

  if (table == nullptr) {
    if (tableCount >= MAX_TABLES) {
      sendJsonError(request, 507, "Table storage full");
      return;
    }

    TableInfo& created = tables[tableCount];
    created.id = id;
    created.name = name;
    created.locked = locked;
    tableCount++;

    saveTables();

    JsonDocument responseDoc;
    JsonObject obj = responseDoc.to<JsonObject>();
    serializeTable(obj, created);

    String response;
    serializeJson(responseDoc, response);
    request->send(201, "application/json", response);
    broadcastTableEvent("tableSaved", created);
    return;
  }

  table->name = name;
  table->locked = locked;

  saveTables();

  JsonDocument responseDoc;
  JsonObject obj = responseDoc.to<JsonObject>();
  serializeTable(obj, *table);

  String response;
  serializeJson(responseDoc, response);
  request->send(200, "application/json", response);
  broadcastTableEvent("tableSaved", *table);
}

void handleTableDelete(AsyncWebServerRequest *request, JsonDocument& doc) {
  String id = doc["id"] | "";

  for (int i = 0; i < tableCount; i++) {
    if (tables[i].id == id) {
      TableInfo deletedTable = tables[i];

      for (int j = i; j < tableCount - 1; j++) {
        tables[j] = tables[j + 1];
      }

      tableCount--;
      saveTables();

      request->send(200, "application/json", "{\"ok\":true}");
      broadcastTableEvent("tableDeleted", deletedTable);
      return;
    }
  }

  sendJsonError(request, 404, "Table not found");
}

void handleSettingsSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  settings.businessName = String((const char*)doc["businessName"]);
  settings.apSsid = String((const char*)doc["apSsid"]);
  settings.apPassword = String((const char*)doc["apPassword"]);
  settings.counterModeEnabled = doc["counterModeEnabled"] | true;

  if (settings.businessName.length() == 0) settings.businessName = "OrderBox Demo";
  if (settings.apSsid.length() == 0) settings.apSsid = WIFI_AP_SSID_DEFAULT;
  if (settings.apPassword.length() < 8) settings.apPassword = WIFI_AP_PASSWORD_DEFAULT;

  saveSettings();

  request->send(200, "application/json", buildSettingsJson());
  broadcastSettingsEvent();
}

void handleResetData(AsyncWebServerRequest *request) {
  LittleFS.remove(PRODUCTS_FILE);
  LittleFS.remove(TABLES_FILE);
  LittleFS.remove(ORDERS_FILE);
  LittleFS.remove(SETTINGS_FILE);

  loadDefaultProducts();
  loadDefaultTables();
  loadDefaultSettings();
  orderCount = 0;
  nextOrderId = 1;
  nextCounterNumber = 1;

  saveProducts();
  saveTables();
  saveOrders();
  saveSettings();

  JsonDocument doc;
  doc["event"] = "dataReset";
  broadcastDoc(doc);

  request->send(200, "application/json", "{\"ok\":true,\"message\":\"Data reset completed\"}");
}

String sanitizeUploadFilename(String filename) {
  filename.replace("\\", "/");

  int slashIndex = filename.lastIndexOf('/');
  if (slashIndex >= 0) {
    filename = filename.substring(slashIndex + 1);
  }

  filename.toLowerCase();

  String clean = "";

  for (size_t i = 0; i < filename.length(); i++) {
    char c = filename.charAt(i);
    bool allowed = (c >= 'a' && c <= 'z') ||
      (c >= '0' && c <= '9') ||
      c == '.' ||
      c == '-' ||
      c == '_';

    clean += allowed ? c : '-';
  }

  if (clean.length() == 0) clean = "product";
  return String(millis()) + "-" + clean;
}

bool isAllowedProductImage(const String& path) {
  return path.endsWith(".jpg") ||
    path.endsWith(".jpeg") ||
    path.endsWith(".png") ||
    path.endsWith(".webp") ||
    path.endsWith(".svg");
}

void handleProductImageUpload(
  AsyncWebServerRequest *request,
  const String& filename,
  size_t index,
  uint8_t *data,
  size_t len,
  bool final
) {
  if (index == 0) {
    if (!LittleFS.exists(PRODUCT_IMAGES_DIR)) {
      LittleFS.mkdir(PRODUCT_IMAGES_DIR);
    }

    String cleanName = sanitizeUploadFilename(filename);
    String path = String(PRODUCT_IMAGES_DIR) + "/" + cleanName;

    if (!isAllowedProductImage(path)) {
      request->send(400, "application/json", "{\"error\":\"Unsupported image type\"}");
      return;
    }

    request->_tempFile = LittleFS.open(path, "w");
    request->_tempObject = new String(path);
  }

  if (!request->_tempFile) return;

  if (index + len > MAX_PRODUCT_IMAGE_SIZE) {
    String *path = static_cast<String*>(request->_tempObject);
    request->_tempFile.close();

    if (path != nullptr) {
      LittleFS.remove(*path);
    }

    request->send(413, "application/json", "{\"error\":\"Image too large\"}");
    return;
  }

  request->_tempFile.write(data, len);

  if (final) {
    request->_tempFile.close();

    String *path = static_cast<String*>(request->_tempObject);
    String responsePath = path != nullptr ? *path : "";

    if (path != nullptr) {
      delete path;
      request->_tempObject = nullptr;
    }

    JsonDocument doc;
    doc["path"] = responsePath;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  }
}
