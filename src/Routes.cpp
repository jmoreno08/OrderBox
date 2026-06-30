#include "App.h"

bool requireAdminAuth(AsyncWebServerRequest *request) {
  if (request->authenticate(ADMIN_AUTH_USER, ADMIN_AUTH_PASSWORD)) {
    return true;
  }

  request->requestAuthentication();
  return false;
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

  server.on("/api/products", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!requireAdminAuth(request)) return;
    request->send(200, "application/json", buildProductsJson(false));
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

  server.on(
    "/api/order/create",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleCreateOrder(request, doc);
    }
  );

  server.on(
    "/api/order/update",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleOrderUpdate(request, doc);
    }
  );

  server.on(
    "/api/order/delete",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleOrderDelete(request, doc);
    }
  );

  server.on(
    "/api/product/save",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleProductSave(request, doc);
    }
  );

  server.on(
    "/api/product/delete",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleProductDelete(request, doc);
    }
  );

  server.on(
    "/api/upload/image",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
      if (!requireAdminAuth(request)) return;
    },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!request->authenticate(ADMIN_AUTH_USER, ADMIN_AUTH_PASSWORD)) {
        if (index == 0) request->requestAuthentication();
        return;
      }

      handleProductImageUpload(request, filename, index, data, len, final);
    }
  );

  server.on(
    "/api/table/save",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleTableSave(request, doc);
    }
  );

  server.on(
    "/api/table/delete",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleTableDelete(request, doc);
    }
  );

  server.on(
    "/api/settings/save",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!requireAdminAuth(request)) return;

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);
      if (error) {
        sendJsonError(request, 400, "Invalid JSON");
        return;
      }
      handleSettingsSave(request, doc);
    }
  );

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
