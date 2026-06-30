#include "App.h"

bool requireAdminAuth(AsyncWebServerRequest *request) {
  if (request->authenticate(ADMIN_AUTH_USER, ADMIN_AUTH_PASSWORD)) return true;
  request->requestAuthentication();
  return false;
}

void handleJsonPost(AsyncWebServerRequest *request, uint8_t *data, size_t len, void (*handler)(AsyncWebServerRequest*, JsonDocument&), bool auth) {
  if (auth && !requireAdminAuth(request)) return;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data, len);
  if (error) {
    sendJsonError(request, 400, "Invalid JSON");
    return;
  }
  handler(request, doc);
}

void setupRoutes() {
  server.on("/admin.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(LittleFS, "/admin.html", "text/html");
  });

  server.on("/kitchen.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(LittleFS, "/kitchen.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/api/public-settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildPublicSettingsJson());
  });

  server.on("/api/menu", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildProductsJson(true));
  });

  server.on("/api/public/extras", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildExtrasJson(true));
  });

  server.on("/api/public/categories", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildCategoriesJson(true));
  });

  server.on("/api/products", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildProductsJson(false));
  });

  server.on("/api/categories", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildCategoriesJson(false));
  });

  server.on("/api/extras", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildExtrasJson(false));
  });

  server.on("/api/tables", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildTablesJson());
  });

  server.on("/api/orders", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildOrdersJson());
  });

  server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildSettingsJson());
  });

  server.on("/api/backup/export", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    JsonDocument doc;
    deserializeJson(doc["settings"], readTextFile(SETTINGS_FILE));
    deserializeJson(doc["products"], buildProductsJson(false));
    deserializeJson(doc["categories"], buildCategoriesJson(false));
    deserializeJson(doc["extras"], buildExtrasJson(false));
    deserializeJson(doc["tables"], buildTablesJson());
    deserializeJson(doc["orders"], buildOrdersJson());
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/api/order/create", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleCreateOrder, false);
    });

  server.on("/api/order/update", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleOrderUpdate, true);
    });

  server.on("/api/order/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleOrderDelete, true);
    });

  server.on("/api/category/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleCategorySave, true);
    });

  server.on("/api/category/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleCategoryDelete, true);
    });

  server.on("/api/product/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleProductSave, true);
    });

  server.on("/api/product/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleProductDelete, true);
    });

  server.on("/api/extra/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleExtraSave, true);
    });

  server.on("/api/extra/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleExtraDelete, true);
    });

  server.on("/api/table/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleTableSave, true);
    });

  server.on("/api/table/delete", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleTableDelete, true);
    });

  server.on("/api/settings/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleJsonPost(request, data, len, handleSettingsSave, true);
    });

  server.on("/api/backup/import", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!request->authenticate(ADMIN_AUTH_USER, ADMIN_AUTH_PASSWORD)) {
        if (index == 0) request->requestAuthentication();
        return;
      }

      if (index == 0) {
        request->_tempObject = new String();
        static_cast<String*>(request->_tempObject)->reserve(total);
      }

      String* body = static_cast<String*>(request->_tempObject);
      if (body == nullptr) {
        sendJsonError(request, 500, "Import buffer error");
        return;
      }

      body->concat((const char*)data, len);

      if (index + len == total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, *body);
        delete body;
        request->_tempObject = nullptr;

        if (error) {
          sendJsonError(request, 400, "Invalid JSON");
          return;
        }

        handleBackupImport(request, doc);
      }
    });

  server.on("/api/upload/image", HTTP_POST, [](AsyncWebServerRequest *request) {
      if (!requireAdminAuth(request)) return;
    },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!request->authenticate(ADMIN_AUTH_USER, ADMIN_AUTH_PASSWORD)) {
        if (index == 0) request->requestAuthentication();
        return;
      }
      handleProductImageUpload(request, filename, index, data, len, final);
    });

  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    handleResetData(request);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    String url = request->url();

    if (url == "/kitchen") {
      if (!requireAdminAuth(request)) return;
      request->redirect("/kitchen.html");
      return;
    }

    if (url == "/admin") {
      if (!requireAdminAuth(request)) return;
      request->redirect("/admin.html");
      return;
    }

    request->send(404, "text/plain", "Not found");
  });
}
