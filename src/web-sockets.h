#ifndef WEB_SOCKETS_H
#define WEB_SOCKETS_H

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include "i2c-multi.h"

using namespace websockets;
void onMessageCallback(WebsocketsMessage message);
void onEventsCallback(WebsocketsEvent event, String data);

void setupWebsockets(String url);
void handleJson(std::string jsonString);

static uint16_t clientId;
static String url;

// Externs
void setPreset(int presetIndex);
void setBrightness(int brightness);
#endif