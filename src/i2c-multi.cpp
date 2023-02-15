#include <Arduino.h>
#include <Wire.h>
// #include <Fonts/FreeSans12pt7b.h>

#include "my_globals.h"
#include "i2c-multi.h"
#include "util.h"
#include "oled-display.h"
#include "wifi-manager.h"
#include "mdns-service.h"
#include "encoders.h"
#include "web-sockets.h"

/* ///////// TODO

 WifiManager
 TaskScheduler
 Reconnect websocket on disconnect
 Future Feature:
    * Send commands over IR?

*/

///////////////////////////// SETUP ///////////////////////////////////////

void setup(void) {

    Serial.begin(115200);
    
    Wire.begin();

    encoderSetup();

    oledSetup();
    
    configureWifi();

    _url = getLumiUrl();

    Serial.println(_url);

    if (_url != "") {
        client.onMessage(onMessageCallback);
        client.onEvent(onEventsCallback);
        client.connect(_url);
    }
    else {
        Serial.println("URL was empty :(");
        for (;;);
    }
}

/////////////////////// LOOP /////////////////////////////

void loop() {
    readEncoders();
    client.poll();
}