#include "App.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Product products[MAX_PRODUCTS];
int productCount = 0;
int nextProductId = 1;

TableInfo tables[MAX_TABLES];
int tableCount = 0;

Order orders[MAX_ORDERS];
int orderCount = 0;
int nextOrderId = 1;
int nextCounterNumber = 1;

AppSettings settings;
