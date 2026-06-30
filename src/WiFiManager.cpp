#include "App.h"

void setupWiFiAP() {
  WiFi.mode(WIFI_AP);
  String effectiveSsid = buildEffectiveApSsid();
  WiFi.softAP(effectiveSsid.c_str(), settings.apPassword.c_str());

  IPAddress ip = WiFi.softAPIP();

  Serial.println();
  Serial.println("================================");
  Serial.println("OrderBox Sprint 2 iniciado");
  Serial.print("Negocio: ");
  Serial.println(settings.businessName);
  Serial.print("SSID: ");
  Serial.println(effectiveSsid);
  Serial.print("SSID base: ");
  Serial.println(settings.apSsid);
  Serial.print("Modulo ID: ");
  Serial.println(getModuleId());
  Serial.print("Password: ");
  Serial.println(settings.apPassword);
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.println("Cliente mesa:     http://192.168.4.1/?table=1");
  Serial.println("Cliente contador: http://192.168.4.1/?mode=counter");
  Serial.println("Cocina:           http://192.168.4.1/kitchen.html");
  Serial.println("Admin:            http://192.168.4.1/admin.html");
  Serial.println("================================");
}

// ===============================
// API helpers
// ===============================
