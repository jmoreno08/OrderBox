#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ======================================================
// OrderBox Sprint 1 - MVP actualizado
// ESP32-S3 + Arduino Framework
//
// Incluye:
// - WiFi AP local
// - Cliente por QR de mesa: /?table=1
// - Cliente por contador: /?mode=counter
// - Panel cocina: /kitchen.html
// - Panel admin: /admin.html
// - Bloqueo/desbloqueo de mesas
// - Activar/desactivar productos
// - Pedidos en RAM
// - WebSocket en tiempo real
// ======================================================

#define WIFI_AP_SSID      "OrderBox"
#define WIFI_AP_PASSWORD  "12345678"

#define MAX_ORDERS 40
#define MAX_ITEMS_PER_ORDER 10
#define MAX_TABLES 20

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

enum OrderStatus {
  RECEIVED,
  PREPARING,
  READY,
  DELIVERED
};

struct Product {
  int id;
  String name;
  float price;
  bool active;
};

struct TableInfo {
  String id;
  bool locked;
};

struct OrderItem {
  int productId;
  String name;
  int qty;
  float price;
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

Product menu[] = {
  {1, "Burger", 18000, true},
  {2, "Pizza", 24000, true},
  {3, "Hot Dog", 14000, true},
  {4, "Coca Cola", 5000, true},
  {5, "Papas", 8000, true}
};

TableInfo tables[MAX_TABLES];
int tableCount = 0;

const int MENU_SIZE = sizeof(menu) / sizeof(menu[0]);

Order orders[MAX_ORDERS];
int orderCount = 0;
int nextOrderId = 1;
int nextCounterNumber = 1;

String statusToString(OrderStatus status) {
  switch (status) {
    case RECEIVED: return "RECEIVED";
    case PREPARING: return "PREPARING";
    case READY: return "READY";
    case DELIVERED: return "DELIVERED";
    default: return "UNKNOWN";
  }
}

OrderStatus stringToStatus(const String& status) {
  if (status == "PREPARING") return PREPARING;
  if (status == "READY") return READY;
  if (status == "DELIVERED") return DELIVERED;
  return RECEIVED;
}

void initTables() {
  tableCount = 8;

  for (int i = 0; i < tableCount; i++) {
    tables[i].id = String(i + 1);
    tables[i].locked = false;
  }
}

Product* findProductById(int id) {
  for (int i = 0; i < MENU_SIZE; i++) {
    if (menu[i].id == id) return &menu[i];
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

void serializeProduct(JsonObject obj, const Product& product) {
  obj["id"] = product.id;
  obj["name"] = product.name;
  obj["price"] = product.price;
  obj["active"] = product.active;
}

void serializeTable(JsonObject obj, const TableInfo& table) {
  obj["id"] = table.id;
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
  }
}

String buildMenuJson(bool onlyActive) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < MENU_SIZE; i++) {
    if (onlyActive && !menu[i].active) continue;

    JsonObject obj = arr.add<JsonObject>();
    serializeProduct(obj, menu[i]);
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

void setupWiFiAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

  IPAddress ip = WiFi.softAPIP();

  Serial.println();
  Serial.println("================================");
  Serial.println("OrderBox WiFi AP iniciado");
  Serial.print("SSID: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("Password: ");
  Serial.println(WIFI_AP_PASSWORD);
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.println("Cliente mesa:     http://192.168.4.1/?table=1");
  Serial.println("Cliente contador: http://192.168.4.1/?mode=counter");
  Serial.println("Cocina:           http://192.168.4.1/kitchen.html");
  Serial.println("Admin:            http://192.168.4.1/admin.html");
  Serial.println("================================");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (!(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)) return;

  data[len] = 0;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, (char*)data);

  if (error) {
    Serial.println("WebSocket JSON inválido");
    return;
  }

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

void setupRoutes() {
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // Cliente: solo productos activos
  server.on("/api/menu", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildMenuJson(true));
  });

  // Admin: todos los productos
  server.on("/api/admin/menu", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildMenuJson(false));
  });

  server.on("/api/tables", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildTablesJson());
  });

  server.on("/api/orders", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildOrdersJson());
  });

  server.on("/api/order", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      if (orderCount >= MAX_ORDERS) {
        request->send(507, "application/json", "{\"error\":\"Order storage full\"}");
        return;
      }

      String sourceType = doc["sourceType"] | "table";
      String table = doc["table"] | "";
      JsonArray items = doc["items"].as<JsonArray>();

      if (sourceType != "table" && sourceType != "counter") {
        request->send(400, "application/json", "{\"error\":\"Invalid sourceType\"}");
        return;
      }

      if (sourceType == "table") {
        if (table.length() == 0) {
          request->send(400, "application/json", "{\"error\":\"Table is required\"}");
          return;
        }

        TableInfo* tableInfo = findTableById(table);
        if (tableInfo == nullptr) {
          request->send(404, "application/json", "{\"error\":\"Table not found\"}");
          return;
        }

        if (tableInfo->locked) {
          request->send(423, "application/json", "{\"error\":\"Table is locked\"}");
          return;
        }
      }

      if (items.size() == 0) {
        request->send(400, "application/json", "{\"error\":\"Items are required\"}");
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
          order.itemCount++;
        }
      }

      if (order.itemCount == 0) {
        request->send(400, "application/json", "{\"error\":\"No valid active items\"}");
        return;
      }

      orderCount++;

      JsonDocument responseDoc;
      JsonObject obj = responseDoc.to<JsonObject>();
      serializeOrder(obj, order);

      String response;
      serializeJson(responseDoc, response);

      request->send(201, "application/json", response);
      broadcastOrderEvent("newOrder", order);

      Serial.printf("Nuevo pedido #%d - tipo %s\n", order.id, order.sourceType.c_str());
    }
  );

  server.on("^\\/api\\/order\\/([0-9]+)$", HTTP_PUT,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      int orderId = request->pathArg(0).toInt();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      Order* order = findOrderById(orderId);

      if (order == nullptr) {
        request->send(404, "application/json", "{\"error\":\"Order not found\"}");
        return;
      }

      String status = doc["status"] | "RECEIVED";
      order->status = stringToStatus(status);

      JsonDocument responseDoc;
      JsonObject obj = responseDoc.to<JsonObject>();
      serializeOrder(obj, *order);

      String response;
      serializeJson(responseDoc, response);

      request->send(200, "application/json", response);
      broadcastOrderEvent("orderUpdated", *order);
    }
  );

  server.on("^\\/api\\/order\\/([0-9]+)$", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    int orderId = request->pathArg(0).toInt();

    for (int i = 0; i < orderCount; i++) {
      if (orders[i].id == orderId) {
        Order deletedOrder = orders[i];

        for (int j = i; j < orderCount - 1; j++) {
          orders[j] = orders[j + 1];
        }

        orderCount--;

        request->send(200, "application/json", "{\"ok\":true}");
        broadcastOrderEvent("orderDeleted", deletedOrder);
        return;
      }
    }

    request->send(404, "application/json", "{\"error\":\"Order not found\"}");
  });

  // Activar/desactivar producto
  server.on("^\\/api\\/product\\/([0-9]+)$", HTTP_PUT,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      int productId = request->pathArg(0).toInt();

      Product* product = findProductById(productId);
      if (product == nullptr) {
        request->send(404, "application/json", "{\"error\":\"Product not found\"}");
        return;
      }

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      product->active = doc["active"] | product->active;

      JsonDocument responseDoc;
      JsonObject obj = responseDoc.to<JsonObject>();
      serializeProduct(obj, *product);

      String response;
      serializeJson(responseDoc, response);

      request->send(200, "application/json", response);
      broadcastProductEvent("productUpdated", *product);
    }
  );

  // Bloquear/desbloquear mesa
  server.on("^\\/api\\/table\\/([^\\/]+)$", HTTP_PUT,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      String tableId = request->pathArg(0);

      TableInfo* table = findTableById(tableId);
      if (table == nullptr) {
        request->send(404, "application/json", "{\"error\":\"Table not found\"}");
        return;
      }

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      table->locked = doc["locked"] | table->locked;

      JsonDocument responseDoc;
      JsonObject obj = responseDoc.to<JsonObject>();
      serializeTable(obj, *table);

      String response;
      serializeJson(responseDoc, response);

      request->send(200, "application/json", response);
      broadcastTableEvent("tableUpdated", *table);
    }
  );

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->url() == "/kitchen") {
      request->redirect("/kitchen.html");
      return;
    }

    if (request->url() == "/admin") {
      request->redirect("/admin.html");
      return;
    }

    request->send(404, "text/plain", "Not found");
  });
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!LittleFS.begin(true)) {
    Serial.println("Error montando LittleFS");
    return;
  }

  initTables();
  setupWiFiAP();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  setupRoutes();

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  ws.cleanupClients();
}
