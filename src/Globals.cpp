#include "App.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Category categories[MAX_CATEGORIES];
int categoryCount = 0;
int nextCategoryId = 1;

Product products[MAX_PRODUCTS];
int productCount = 0;
int nextProductId = 1;

Extra extras[MAX_EXTRAS];
int extraCount = 0;
int nextExtraId = 1;

TableInfo tables[MAX_TABLES];
int tableCount = 0;

Order orders[MAX_ORDERS];
int orderCount = 0;
int nextOrderId = 1;
int nextCounterNumber = 1;

AppSettings settings;
BusinessConfig business;
