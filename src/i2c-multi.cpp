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

    String url = getLumiUrl();
    Serial.println(url);
    
    if (url != "") {
        setupWebsockets(url);
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