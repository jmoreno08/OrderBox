#include "App.h"

float orderTotal(const Order& order) {
  float total = 0;
  for (int i = 0; i < order.itemCount; i++) {
    total += order.items[i].price * order.items[i].qty;
    for (int j = 0; j < order.items[i].extraCount; j++) {
      total += order.items[i].extras[j].price * order.items[i].qty;
    }
  }
  return total;
}

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
    TableInfo* tableInfo = findTableById(table);
    if (table.length() == 0 || tableInfo == nullptr) {
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
  order.notes = doc["notes"] | "";

  for (JsonObject item : items) {
    if (order.itemCount >= MAX_ITEMS_PER_ORDER) break;

    int productId = item["productId"] | 0;
    int qty = item["qty"] | 0;
    Product* product = findProductById(productId);
    Category* category = product != nullptr ? findCategoryById(product->categoryId) : nullptr;

    if (product == nullptr || !product->active || category == nullptr || !category->active || qty <= 0) continue;

    OrderItem& orderItem = order.items[order.itemCount];
    orderItem.productId = product->id;
    orderItem.name = product->name;
    orderItem.qty = qty;
    orderItem.price = product->price;
    orderItem.image = product->image;
    orderItem.notes = item["notes"] | "";
    orderItem.extraCount = 0;

    for (JsonVariant extraValue : item["extras"].as<JsonArray>()) {
      if (orderItem.extraCount >= MAX_EXTRAS_PER_ITEM) break;
      int extraId = extraValue["id"] | extraValue.as<int>();
      Extra* extra = findExtraById(extraId);
      if (extra == nullptr || !extra->active || extra->productId != product->id) continue;
      orderItem.extras[orderItem.extraCount] = *extra;
      orderItem.extraCount++;
    }

    order.itemCount++;
  }

  if (order.itemCount == 0) {
    sendJsonError(request, 400, "No valid active items");
    return;
  }

  orderCount++;
  saveOrders();
  printOrderTicket(order);

  JsonDocument responseDoc;
  JsonObject obj = responseDoc.to<JsonObject>();
  serializeOrder(obj, order);

  String response;
  serializeJson(responseDoc, response);
  request->send(201, "application/json", response);
  broadcastOrderEvent("newOrder", order);
}

void handleOrderUpdate(AsyncWebServerRequest *request, JsonDocument& doc) {
  Order* order = findOrderById(doc["id"] | 0);
  if (order == nullptr) {
    sendJsonError(request, 404, "Order not found");
    return;
  }

  String requestedStatus = doc["status"] | "RECEIVED";

  if (requestedStatus == "CANCELLED" && order->status != RECEIVED) {
    sendJsonError(request, 409, "Only received orders can be cancelled");
    return;
  }

  order->status = stringToStatus(requestedStatus);
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
      for (int j = i; j < orderCount - 1; j++) orders[j] = orders[j + 1];
      orderCount--;
      saveOrders();
      request->send(200, "application/json", "{\"ok\":true}");
      broadcastOrderEvent("orderDeleted", deletedOrder);
      return;
    }
  }

  sendJsonError(request, 404, "Order not found");
}

void handleCategorySave(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  String name = doc["name"] | "";
  bool active = doc["active"] | true;

  if (name.length() == 0) {
    sendJsonError(request, 400, "Category name is required");
    return;
  }

  Category* category = id > 0 ? findCategoryById(id) : nullptr;
  if (category == nullptr) {
    if (categoryCount >= MAX_CATEGORIES) {
      sendJsonError(request, 507, "Category storage full");
      return;
    }
    category = &categories[categoryCount++];
    category->id = nextCategoryId++;
  }

  category->name = name;
  category->active = active;
  saveCategories();
  request->send(200, "application/json", buildCategoriesJson(false));
}

void handleCategoryDelete(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  for (int i = 0; i < categoryCount; i++) {
    if (categories[i].id == id) {
      for (int j = i; j < categoryCount - 1; j++) categories[j] = categories[j + 1];
      categoryCount--;
      saveCategories();
      request->send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  sendJsonError(request, 404, "Category not found");
}

void handleProductSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  String name = doc["name"] | "";
  int categoryId = doc["categoryId"] | 0;
  Category* category = findCategoryById(categoryId);

  if (name.length() == 0 || category == nullptr) {
    sendJsonError(request, 400, "Product name and category are required");
    return;
  }

  Product* product = id > 0 ? findProductById(id) : nullptr;
  if (product == nullptr) {
    if (productCount >= MAX_PRODUCTS) {
      sendJsonError(request, 507, "Product storage full");
      return;
    }
    product = &products[productCount++];
    product->id = nextProductId++;
  }

  product->name = name;
  product->price = doc["price"] | 0;
  product->active = doc["active"] | true;
  product->categoryId = categoryId;
  product->category = category->name;
  product->description = doc["description"] | "";
  product->image = doc["image"] | "/img/placeholder.svg";
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
      for (int j = i; j < productCount - 1; j++) products[j] = products[j + 1];
      productCount--;
      saveProducts();
      request->send(200, "application/json", "{\"ok\":true}");
      broadcastProductEvent("productDeleted", deletedProduct);
      return;
    }
  }
  sendJsonError(request, 404, "Product not found");
}

void handleExtraSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  int productId = doc["productId"] | 0;
  String name = doc["name"] | "";

  if (name.length() == 0 || findProductById(productId) == nullptr) {
    sendJsonError(request, 400, "Extra name and product are required");
    return;
  }

  Extra* extra = id > 0 ? findExtraById(id) : nullptr;
  if (extra == nullptr) {
    if (extraCount >= MAX_EXTRAS) {
      sendJsonError(request, 507, "Extra storage full");
      return;
    }
    extra = &extras[extraCount++];
    extra->id = nextExtraId++;
  }

  extra->productId = productId;
  extra->name = name;
  extra->price = doc["price"] | 0;
  extra->active = doc["active"] | true;
  saveExtras();
  request->send(200, "application/json", buildExtrasJson(false));
}

void handleExtraDelete(AsyncWebServerRequest *request, JsonDocument& doc) {
  int id = doc["id"] | 0;
  for (int i = 0; i < extraCount; i++) {
    if (extras[i].id == id) {
      for (int j = i; j < extraCount - 1; j++) extras[j] = extras[j + 1];
      extraCount--;
      saveExtras();
      request->send(200, "application/json", "{\"ok\":true}");
      return;
    }
  }
  sendJsonError(request, 404, "Extra not found");
}

void handleTableSave(AsyncWebServerRequest *request, JsonDocument& doc) {
  String id = doc["id"] | "";
  String name = doc["name"] | "";
  bool locked = doc["locked"] | false;

  if (id.length() == 0) {
    sendJsonError(request, 400, "Table id is required");
    return;
  }
  if (name.length() == 0) name = "Mesa " + id;

  TableInfo* table = findTableById(id);
  if (table == nullptr) {
    if (tableCount >= MAX_TABLES) {
      sendJsonError(request, 507, "Table storage full");
      return;
    }
    table = &tables[tableCount++];
    table->id = id;
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
      for (int j = i; j < tableCount - 1; j++) tables[j] = tables[j + 1];
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
  LittleFS.remove(CATEGORIES_FILE);
  LittleFS.remove(EXTRAS_FILE);
  LittleFS.remove(TABLES_FILE);
  LittleFS.remove(ORDERS_FILE);
  LittleFS.remove(SETTINGS_FILE);

  loadDefaultSettings();
  loadDefaultCategories();
  loadDefaultProducts();
  loadDefaultExtras();
  loadDefaultTables();
  orderCount = 0;
  nextOrderId = 1;
  nextCounterNumber = 1;

  saveSettings();
  saveCategories();
  saveProducts();
  saveExtras();
  saveTables();
  saveOrders();

  JsonDocument doc;
  doc["event"] = "dataReset";
  broadcastDoc(doc);
  request->send(200, "application/json", "{\"ok\":true,\"message\":\"Data reset completed\"}");
}

void handleBackupImport(AsyncWebServerRequest *request, JsonDocument& doc) {
  if (!doc["settings"].is<JsonObject>() || !doc["products"].is<JsonArray>() ||
      !doc["categories"].is<JsonArray>() || !doc["extras"].is<JsonArray>() ||
      !doc["tables"].is<JsonArray>() || !doc["orders"].is<JsonArray>()) {
    sendJsonError(request, 400, "Invalid backup JSON");
    return;
  }

  String output;
  serializeJson(doc["settings"], output); writeTextFile(SETTINGS_FILE, output); output = "";
  serializeJson(doc["categories"], output); writeTextFile(CATEGORIES_FILE, output); output = "";
  serializeJson(doc["products"], output); writeTextFile(PRODUCTS_FILE, output); output = "";
  serializeJson(doc["extras"], output); writeTextFile(EXTRAS_FILE, output); output = "";
  serializeJson(doc["tables"], output); writeTextFile(TABLES_FILE, output); output = "";
  serializeJson(doc["orders"], output); writeTextFile(ORDERS_FILE, output);

  loadAllData();
  request->send(200, "application/json", "{\"ok\":true}");
}

String sanitizeUploadFilename(String filename) {
  filename.replace("\\", "/");
  int slashIndex = filename.lastIndexOf('/');
  if (slashIndex >= 0) filename = filename.substring(slashIndex + 1);
  filename.toLowerCase();

  String clean = "";
  for (size_t i = 0; i < filename.length(); i++) {
    char c = filename.charAt(i);
    bool allowed = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_';
    clean += allowed ? c : '-';
  }

  if (clean.length() == 0) clean = "product";
  return String(millis()) + "-" + clean;
}

bool isAllowedProductImage(const String& path) {
  return path.endsWith(".jpg") || path.endsWith(".jpeg") || path.endsWith(".png") || path.endsWith(".webp") || path.endsWith(".svg");
}

void handleProductImageUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    if (!LittleFS.exists(PRODUCT_IMAGES_DIR)) LittleFS.mkdir(PRODUCT_IMAGES_DIR);

    String path = String(PRODUCT_IMAGES_DIR) + "/" + sanitizeUploadFilename(filename);
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
    if (path != nullptr) LittleFS.remove(*path);
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
