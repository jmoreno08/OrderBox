#include "App.h"

bool writeTextFile(const char* path, const String& content) {
  File file = LittleFS.open(path, "w");
  if (!file) return false;
  file.print(content);
  file.close();
  return true;
}

String readTextFile(const char* path) {
  File file = LittleFS.open(path, "r");
  if (!file) return "";
  String content = file.readString();
  file.close();
  return content;
}

void saveProducts() {
  writeTextFile(PRODUCTS_FILE, buildProductsJson(false));
}

void saveTables() {
  writeTextFile(TABLES_FILE, buildTablesJson());
}

void saveOrders() {
  writeTextFile(ORDERS_FILE, buildOrdersJson());
}

void saveSettings() {
  JsonDocument doc;
  doc["businessName"] = settings.businessName;
  doc["apSsid"] = settings.apSsid;
  doc["apPassword"] = settings.apPassword;
  doc["counterModeEnabled"] = settings.counterModeEnabled;

  String response;
  serializeJson(doc, response);
  writeTextFile(SETTINGS_FILE, response);
}

void loadDefaultProducts() {
  productCount = 6;
  products[0] = {1, "Hamburguesa Clásica", 25000, true, "Hamburguesas", "Carne, queso, lechuga, tomate y salsa especial.", "/img/burger.svg"};
  products[1] = {2, "Pizza Pepperoni", 28000, true, "Pizzas", "Masa artesanal, queso mozzarella y pepperoni.", "/img/pizza.svg"};
  products[2] = {3, "Papas Fritas", 8000, true, "Entradas", "Porción grande, crocante y dorada.", "/img/fries.svg"};
  products[3] = {4, "Coca Cola", 5000, true, "Bebidas", "Bebida fría 350 ml.", "/img/soda.svg"};
  products[4] = {5, "Ensalada César", 22000, true, "Ensaladas", "Lechuga, pollo, crutones y aderezo César.", "/img/salad.svg"};
  products[5] = {6, "Cupcake", 9000, true, "Postres", "Postre individual con crema suave.", "/img/cupcake.svg"};
  nextProductId = 7;
}

void loadDefaultTables() {
  tableCount = 8;
  for (int i = 0; i < tableCount; i++) {
    tables[i].id = String(i + 1);
    tables[i].name = "Mesa " + String(i + 1);
    tables[i].locked = false;
  }
}

void loadDefaultSettings() {
  settings.businessName = "OrderBox Demo";
  settings.apSsid = WIFI_AP_SSID_DEFAULT;
  settings.apPassword = WIFI_AP_PASSWORD_DEFAULT;
  settings.counterModeEnabled = true;
}

void loadProducts() {
  String content = readTextFile(PRODUCTS_FILE);

  if (content.length() == 0) {
    loadDefaultProducts();
    saveProducts();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultProducts();
    saveProducts();
    return;
  }

  productCount = 0;
  nextProductId = 1;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (productCount >= MAX_PRODUCTS) break;

    products[productCount].id = obj["id"] | nextProductId;
    products[productCount].name = String((const char*)obj["name"]);
    products[productCount].price = obj["price"] | 0;
    products[productCount].active = obj["active"] | true;
    products[productCount].category = obj["category"] | "General";
    products[productCount].description = obj["description"] | "";
    products[productCount].image = obj["image"] | "/img/placeholder.svg";

    if (products[productCount].id >= nextProductId) {
      nextProductId = products[productCount].id + 1;
    }

    productCount++;
  }

  if (productCount == 0) {
    loadDefaultProducts();
    saveProducts();
  }
}

void loadTables() {
  String content = readTextFile(TABLES_FILE);

  if (content.length() == 0) {
    loadDefaultTables();
    saveTables();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultTables();
    saveTables();
    return;
  }

  tableCount = 0;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (tableCount >= MAX_TABLES) break;

    tables[tableCount].id = String((const char*)obj["id"]);
    tables[tableCount].name = String((const char*)obj["name"]);
    tables[tableCount].locked = obj["locked"] | false;

    if (tables[tableCount].id.length() == 0) {
      tables[tableCount].id = String(tableCount + 1);
    }

    if (tables[tableCount].name.length() == 0) {
      tables[tableCount].name = "Mesa " + tables[tableCount].id;
    }

    tableCount++;
  }

  if (tableCount == 0) {
    loadDefaultTables();
    saveTables();
  }
}

void loadOrders() {
  String content = readTextFile(ORDERS_FILE);

  if (content.length() == 0) {
    orderCount = 0;
    nextOrderId = 1;
    nextCounterNumber = 1;
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    orderCount = 0;
    nextOrderId = 1;
    nextCounterNumber = 1;
    return;
  }

  orderCount = 0;
  nextOrderId = 1;
  nextCounterNumber = 1;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (orderCount >= MAX_ORDERS) break;

    Order& order = orders[orderCount];
    order.id = obj["id"] | nextOrderId;
    order.sourceType = String((const char*)obj["sourceType"]);
    order.table = String((const char*)obj["table"]);
    order.counterNumber = obj["counterNumber"] | 0;
    order.status = stringToStatus(String((const char*)obj["status"]));
    order.createdAt = obj["createdAt"] | 0;
    order.itemCount = 0;

    JsonArray items = obj["items"].as<JsonArray>();

    for (JsonObject item : items) {
      if (order.itemCount >= MAX_ITEMS_PER_ORDER) break;

      order.items[order.itemCount].productId = item["productId"] | 0;
      order.items[order.itemCount].name = String((const char*)item["name"]);
      order.items[order.itemCount].qty = item["qty"] | 0;
      order.items[order.itemCount].price = item["price"] | 0;
      order.items[order.itemCount].image = item["image"] | "/img/placeholder.svg";
      order.itemCount++;
    }

    if (order.id >= nextOrderId) nextOrderId = order.id + 1;
    if (order.counterNumber >= nextCounterNumber) nextCounterNumber = order.counterNumber + 1;

    orderCount++;
  }
}

void loadSettings() {
  String content = readTextFile(SETTINGS_FILE);

  if (content.length() == 0) {
    loadDefaultSettings();
    saveSettings();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultSettings();
    saveSettings();
    return;
  }

  settings.businessName = String((const char*)doc["businessName"]);
  settings.apSsid = String((const char*)doc["apSsid"]);
  settings.apPassword = String((const char*)doc["apPassword"]);
  settings.counterModeEnabled = doc["counterModeEnabled"] | true;

  if (settings.businessName.length() == 0) settings.businessName = "OrderBox Demo";
  if (settings.apSsid.length() == 0) settings.apSsid = WIFI_AP_SSID_DEFAULT;
  if (settings.apPassword.length() < 8) settings.apPassword = WIFI_AP_PASSWORD_DEFAULT;
}

void loadAllData() {
  loadProducts();
  loadTables();
  loadOrders();
  loadSettings();
}
