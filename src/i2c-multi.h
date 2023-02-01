#include <Arduino.h>
#include <Wire.h>
#include <i2cEncoderLibV2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> 
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
// #include <Fonts/FreeSans12pt7b.h>
#include <ESPmDNS.h>

// void start_mdns_service();

void testdrawchar(void);
void displayParameter(int text, int number);
void drawCentreString(const String &buf, int y);
void sendParameter(int id, int value);
std::string timeToString();
void handleJson(std::string jsonString);
void setPreset(int presetIndex);
void setBrightness(int brightness);


