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

void saveCategories() {
  writeTextFile(CATEGORIES_FILE, buildCategoriesJson(false));
}

void saveProducts() {
  writeTextFile(PRODUCTS_FILE, buildProductsJson(false));
}

void saveExtras() {
  writeTextFile(EXTRAS_FILE, buildExtrasJson(false));
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

void saveBusiness() {
  writeTextFile(BUSINESS_FILE, buildBusinessJson());
}

int ensureCategoryByName(const String& name) {
  String safeName = name.length() ? name : "General";

  for (int i = 0; i < categoryCount; i++) {
    if (categories[i].name == safeName) return categories[i].id;
  }

  if (categoryCount >= MAX_CATEGORIES) return categories[0].id;

  Category& category = categories[categoryCount];
  category.id = nextCategoryId++;
  category.name = safeName;
  category.active = true;
  categoryCount++;
  return category.id;
}

void loadDefaultCategories() {
  categoryCount = 6;
  categories[0] = {1, "Hamburguesas", true};
  categories[1] = {2, "Pizzas", true};
  categories[2] = {3, "Entradas", true};
  categories[3] = {4, "Bebidas", true};
  categories[4] = {5, "Ensaladas", true};
  categories[5] = {6, "Postres", true};
  nextCategoryId = 7;
}

void loadDefaultProducts() {
  productCount = 6;
  products[0] = {1, "Hamburguesa Clasica", 25000, true, 1, "Hamburguesas", "Carne, queso, lechuga, tomate y salsa especial.", "/img/burger.svg"};
  products[1] = {2, "Pizza Pepperoni", 28000, true, 2, "Pizzas", "Masa artesanal, queso mozzarella y pepperoni.", "/img/pizza.svg"};
  products[2] = {3, "Papas Fritas", 8000, true, 3, "Entradas", "Porcion grande, crocante y dorada.", "/img/fries.svg"};
  products[3] = {4, "Coca Cola", 5000, true, 4, "Bebidas", "Bebida fria 350 ml.", "/img/soda.svg"};
  products[4] = {5, "Ensalada Cesar", 22000, true, 5, "Ensaladas", "Lechuga, pollo, crutones y aderezo Cesar.", "/img/salad.svg"};
  products[5] = {6, "Cupcake", 9000, true, 6, "Postres", "Postre individual con crema suave.", "/img/cupcake.svg"};
  nextProductId = 7;
}

void loadDefaultExtras() {
  extraCount = 6;
  extras[0] = {1, 1, "Tocineta", 4000, true};
  extras[1] = {2, 1, "Queso extra", 3000, true};
  extras[2] = {3, 2, "Borde de queso", 6000, true};
  extras[3] = {4, 2, "Pepperoni extra", 5000, true};
  extras[4] = {5, 3, "Salsa cheddar", 2500, true};
  extras[5] = {6, 4, "Hielo extra", 0, true};
  nextExtraId = 7;
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

void loadDefaultBusiness() {
  business.businessName = settings.businessName.length() ? settings.businessName : "OrderBox Demo";
  business.logo = "";
  business.primaryColor = "#5b4bff";
  business.address = "";
  business.phone = "";
  business.currency = "$";
  business.taxEnabled = false;
  business.taxRate = 0;
  business.serviceTipEnabled = false;
  business.serviceTipRate = 0;
}

void loadCategories() {
  String content = readTextFile(CATEGORIES_FILE);

  if (content.length() == 0) {
    loadDefaultCategories();
    saveCategories();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultCategories();
    saveCategories();
    return;
  }

  categoryCount = 0;
  nextCategoryId = 1;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (categoryCount >= MAX_CATEGORIES) break;
    categories[categoryCount].id = obj["id"] | nextCategoryId;
    categories[categoryCount].name = String((const char*)obj["name"]);
    categories[categoryCount].active = obj["active"] | true;
    if (categories[categoryCount].name.length() == 0) categories[categoryCount].name = "General";
    if (categories[categoryCount].id >= nextCategoryId) nextCategoryId = categories[categoryCount].id + 1;
    categoryCount++;
  }

  if (categoryCount == 0) {
    loadDefaultCategories();
    saveCategories();
  }
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

    String categoryName = String((const char*)(obj["category"] | "General"));
    products[productCount].id = obj["id"] | nextProductId;
    products[productCount].name = String((const char*)obj["name"]);
    products[productCount].price = obj["price"] | 0;
    products[productCount].active = obj["active"] | true;
    products[productCount].categoryId = obj["categoryId"] | ensureCategoryByName(categoryName);
    products[productCount].category = categoryName;
    products[productCount].description = obj["description"] | "";
    products[productCount].image = obj["image"] | "/img/placeholder.svg";

    if (products[productCount].id >= nextProductId) nextProductId = products[productCount].id + 1;
    productCount++;
  }

  if (productCount == 0) {
    loadDefaultProducts();
    saveProducts();
  }
}

void loadExtras() {
  String content = readTextFile(EXTRAS_FILE);

  if (content.length() == 0) {
    loadDefaultExtras();
    saveExtras();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultExtras();
    saveExtras();
    return;
  }

  extraCount = 0;
  nextExtraId = 1;

  for (JsonObject obj : doc.as<JsonArray>()) {
    if (extraCount >= MAX_EXTRAS) break;
    extras[extraCount].id = obj["id"] | nextExtraId;
    extras[extraCount].productId = obj["productId"] | 0;
    extras[extraCount].name = String((const char*)obj["name"]);
    extras[extraCount].price = obj["price"] | 0;
    extras[extraCount].active = obj["active"] | true;
    if (extras[extraCount].id >= nextExtraId) nextExtraId = extras[extraCount].id + 1;
    extraCount++;
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
    if (tables[tableCount].id.length() == 0) tables[tableCount].id = String(tableCount + 1);
    if (tables[tableCount].name.length() == 0) tables[tableCount].name = "Mesa " + tables[tableCount].id;
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
    order.readyAt = obj["readyAt"] | 0;
    order.notes = obj["notes"] | "";
    order.cancelReason = obj["cancelReason"] | "";
    order.itemCount = 0;

    for (JsonObject item : obj["items"].as<JsonArray>()) {
      if (order.itemCount >= MAX_ITEMS_PER_ORDER) break;

      OrderItem& orderItem = order.items[order.itemCount];
      orderItem.productId = item["productId"] | 0;
      orderItem.name = String((const char*)item["name"]);
      orderItem.qty = item["qty"] | 0;
      orderItem.price = item["price"] | 0;
      orderItem.image = item["image"] | "/img/placeholder.svg";
      orderItem.notes = item["notes"] | "";
      orderItem.extraCount = 0;

      for (JsonObject extra : item["extras"].as<JsonArray>()) {
        if (orderItem.extraCount >= MAX_EXTRAS_PER_ITEM) break;
        orderItem.extras[orderItem.extraCount].id = extra["id"] | 0;
        orderItem.extras[orderItem.extraCount].productId = extra["productId"] | orderItem.productId;
        orderItem.extras[orderItem.extraCount].name = String((const char*)extra["name"]);
        orderItem.extras[orderItem.extraCount].price = extra["price"] | 0;
        orderItem.extras[orderItem.extraCount].active = extra["active"] | true;
        orderItem.extraCount++;
      }

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

void loadBusiness() {
  String content = readTextFile(BUSINESS_FILE);

  if (content.length() == 0) {
    loadDefaultBusiness();
    saveBusiness();
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, content)) {
    loadDefaultBusiness();
    saveBusiness();
    return;
  }

  business.businessName = String((const char*)doc["businessName"]);
  business.logo = String((const char*)doc["logo"]);
  business.primaryColor = String((const char*)doc["primaryColor"]);
  business.address = String((const char*)doc["address"]);
  business.phone = String((const char*)doc["phone"]);
  business.currency = String((const char*)doc["currency"]);
  business.taxEnabled = doc["taxEnabled"] | false;
  business.taxRate = doc["taxRate"] | 0;
  business.serviceTipEnabled = doc["serviceTipEnabled"] | false;
  business.serviceTipRate = doc["serviceTipRate"] | 0;

  if (business.businessName.length() == 0) business.businessName = settings.businessName;
  if (business.primaryColor.length() == 0) business.primaryColor = "#5b4bff";
  if (business.currency.length() == 0) business.currency = "$";
}

void loadAllData() {
  loadSettings();
  loadBusiness();
  loadCategories();
  loadProducts();
  loadExtras();
  loadTables();
  loadOrders();
}
