#include "wifi-manager.h"
#include "i2c-multi.h"

void configureWifi() {
  constexpr const char* SGN = "configureWifi()";

  WiFiManager wifiManager;

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  // wifiManager.resetSettings();     // Enable for testing - forgets credentials every time
  wifiManager.setAPClientCheck(true); // avoid timeout if client connected to softap
  wifiManager.setClass("invert");     // set dark theme 

  String myApUrl = String("WIFI:S:") + RB_WIFI_AP + ";T:nopass;P:;;";
  drawQrCode(myApUrl.c_str(), MESSAGE_CONFIGURE_WIFI);

  // autoConnect() is confusing as fuck: automatically connect using saved credentials if they exist
  // If connection fails it starts an access point with the specified name
  boolean configuredWifi = wifiManager.autoConnect(RB_WIFI_AP);

  if (!configuredWifi) {
    Serial.println("Wifi Manager Failed!");
    delay(5000);
    ESP.restart();
  }
}
