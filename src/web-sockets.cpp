#include "my_globals.h"
#include "web-sockets.h"

void setupWebsockets(String wsUrl) {
    // Generate our clientId so we can ignore JSON the server echo's back
    clientId = random(999999);
    url = wsUrl;

    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    client.connect(url);
}

void sendParameter(String encoderName, int value, boolean immediateSend) {
    if (!client.available()) {
        return;
    }
    if (immediateSend) {
        DynamicJsonDocument object(1024);
        object[encoderName] = value;
        object["clientId"] = clientId;
        String jsonString;
        serializeJsonPretty(object, jsonString);
        Serial.println(jsonString);
        client.send(jsonString);
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
        delay(1000);
        client.connect(url);
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
        // Serial.printf("%s: %s: deserializeJson() failed\n", timeToString().c_str(), SGN);
        Serial.printf("%s: %s: deserializeJson() failed\n", "", SGN);
    }

    uint16_t receivedClientId = doc["clientId"];
    if (receivedClientId != clientId) {

        if (doc["preset"]) {
            int presetIndex = doc["preset"];
            setPreset(presetIndex);
        }

        if (doc["brightness"]) {
            int brightness = doc["brightness"];
            setBrightness(brightness);
        }
    }
}