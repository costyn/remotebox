#ifndef MDNS_SERVICE_H
#define MDNS_SERVICE_H

#include <Arduino.h>
#include <ESPmDNS.h>

// https://github.com/gilmaimon/ArduinoWebsockets
#define WEBSOCKET_SERVER_PORT 80 // Enter server port
#define WEBSOCKET_SERVER_PATH "/ws" // Websocket path

String getLumiUrl();

#endif