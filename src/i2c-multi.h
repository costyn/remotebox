#ifndef I2C_MULTI_H
#define I2C_MULTI_H

#include <Arduino.h>

std::string timeToString();

void displayParameter(String encoderName, int value, boolean confirmWithClick);
void sendParameter(String encoderName, int value, boolean immediateSend);
void drawQrCode(const char* qrStr, const char* lines[]);

#define RB_WIFI_AP "RemoteBox Setup"

static const char* PROGMEM MESSAGE_CONFIGURE_WIFI[4] = { "Scan QR", "to setup", "WiFi", "" };
// const char* PROGMEM MESSAGE_OPEN_WEBAPP[4] = { "Scan QR", "to open", "Lumifera", "webapp" };
// const char* PROGMEM MESSAGE_JOIN_FAILED[4] = { "Joining", "configured", "WiFi failed", "" };
// const char* PROGMEM MESSAGE_JOIN_SOFT_AP[4] =  { "Scan QR", "to join", "Lumifera", "WiFi AP" };
// const char* PROGMEM MESSAGE_LOCAL_WIFI_CONN[4] = { "Lumifera", "connected", "to local", "WiFi" };

#endif