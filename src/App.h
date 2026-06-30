#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define WIFI_AP_SSID_DEFAULT      "OrderBox"
#define WIFI_AP_PASSWORD_DEFAULT  "12345678"
#define ADMIN_AUTH_USER           "admin"
#define ADMIN_AUTH_PASSWORD       "12345678"

#define MAX_PRODUCTS 40
#define MAX_TABLES 30
#define MAX_ORDERS 60
#define MAX_ITEMS_PER_ORDER 12

#define PRODUCTS_FILE "/products.json"
#define TABLES_FILE   "/tables.json"
#define ORDERS_FILE   "/orders.json"
#define SETTINGS_FILE "/settings.json"
#define PRODUCT_IMAGES_DIR "/img/products"
#define MAX_PRODUCT_IMAGE_SIZE 120000

enum OrderStatus {
  RECEIVED,
  PREPARING,
  READY,
  DELIVERED,
  CANCELLED
};

struct Product {
  int id;
  String name;
  float price;
  bool active;
  String category;
  String description;
  String image;
};

struct TableInfo {
  String id;
  String name;
  bool locked;
};

struct OrderItem {
  int productId;
  String name;
  int qty;
  float price;
  String image;
};

struct Order {
  int id;
  String sourceType; // table | counter
  String table;
  int counterNumber;
  OrderItem items[MAX_ITEMS_PER_ORDER];
  int itemCount;
  OrderStatus status;
  unsigned long createdAt;
};

struct AppSettings {
  String businessName;
  String apSsid;
  String apPassword;
  bool counterModeEnabled;
};

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern Product products[MAX_PRODUCTS];
extern int productCount;
extern int nextProductId;
extern TableInfo tables[MAX_TABLES];
extern int tableCount;
extern Order orders[MAX_ORDERS];
extern int orderCount;
extern int nextOrderId;
extern int nextCounterNumber;
extern AppSettings settings;

String statusToString(OrderStatus status);
OrderStatus stringToStatus(const String& status);
String readBody(AsyncWebServerRequest *request);
Product* findProductById(int id);
TableInfo* findTableById(const String& id);
Order* findOrderById(int id);
void sendJsonError(AsyncWebServerRequest *request, int code, const String& message);

void serializeProduct(JsonObject obj, const Product& product);
void serializeTable(JsonObject obj, const TableInfo& table);
void serializeOrder(JsonObject obj, const Order& order);
String buildProductsJson(bool onlyActive);
String buildTablesJson();
String buildOrdersJson();
String buildSettingsJson();
String buildPublicSettingsJson();

bool writeTextFile(const char* path, const String& content);
String readTextFile(const char* path);
void saveProducts();
void saveTables();
void saveOrders();
void saveSettings();
void loadDefaultProducts();
void loadDefaultTables();
void loadDefaultSettings();
void loadProducts();
void loadTables();
void loadOrders();
void loadSettings();
void loadAllData();

void broadcastDoc(JsonDocument& doc);
void broadcastOrderEvent(const String& eventName, const Order& order);
void broadcastProductEvent(const String& eventName, const Product& product);
void broadcastTableEvent(const String& eventName, const TableInfo& table);
void broadcastSettingsEvent();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void setupWiFiAP();

void handleCreateOrder(AsyncWebServerRequest *request, JsonDocument& doc);
void handleOrderUpdate(AsyncWebServerRequest *request, JsonDocument& doc);
void handleOrderDelete(AsyncWebServerRequest *request, JsonDocument& doc);
void handleProductSave(AsyncWebServerRequest *request, JsonDocument& doc);
void handleProductDelete(AsyncWebServerRequest *request, JsonDocument& doc);
void handleTableSave(AsyncWebServerRequest *request, JsonDocument& doc);
void handleTableDelete(AsyncWebServerRequest *request, JsonDocument& doc);
void handleSettingsSave(AsyncWebServerRequest *request, JsonDocument& doc);
void handleResetData(AsyncWebServerRequest *request);
void handleProductImageUpload(
  AsyncWebServerRequest *request,
  const String& filename,
  size_t index,
  uint8_t *data,
  size_t len,
  bool final
);

void setupRoutes();
