#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
// #include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>

#include "i2c-multi.h"
#include "util.h"
#include "oled-display.h"
#include "wifi-manager.h"
#include "mdns-service.h"
#include "encoders.h"

/* ///////// TODO

 WifiManager
 TaskScheduler
 Move Websockets to own files
 Reconnect websocket on disconnect
 Future Feature:
    * Send commands over IR?

*/

String _url = "";

///////////////////// NETWORK STUFF ///////////////////////


using namespace websockets;
WebsocketsClient client;
void onMessageCallback(WebsocketsMessage message);
void onEventsCallback(WebsocketsEvent event, String data);


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



//////////////////////// WEBSOCKETS ////////////////////////////////

void sendParameter(String encoderName, int value, boolean immediateSend) {
    if (!client.available()) {
        return;
    }
    if (immediateSend) {
        String json = "{" + encoderName + ": " + value + "}\n";
        Serial.println(json);
        client.send(json);
    }
}

using namespace websockets;
void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
    handleJson((message.data()).c_str());
}

using namespace websockets;
void onEventsCallback(WebsocketsEvent event, String data) {
    Serial.println("websocket event");
    
    if (event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Websocket: Connnection Opened");
        // display.println("Websocket: ");
        // display.println(" Connected!");
        // display.println();
        // display.println("You may begin!");
        // display.display();
    }
    else if (event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
        // display.clearDisplay();
        // display.setCursor(0,0);
        // display.println("Websocket: Closed");
        // display.println();
        // display.display();
        client.connect(_url);
    }
    else if (event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}


void handleJson(std::string jsonString) {
    constexpr const char* SGN = "handleJson()";

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.printf("%s: %s: deserializeJson() failed\n", timeToString().c_str(), SGN);
  }

  if (doc["preset"]) {
    int presetIndex = doc["preset"];
    setPreset(presetIndex);
  }

  if (doc["brightness"]) {
    int brightness = doc["brightness"];
    setBrightness(brightness);
  }
}
