#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> 
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
// #include <Fonts/FreeSans12pt7b.h>
#include <qrcode.h>

#include "i2c-multi.h"
#include "mdns-service.h"
#include "encoders.h"
#include "oled-display.h"

/////////// TODO
///// WifiManager
///// TaskScheduler


const int IntPin = A3; // INT pins, change according to your board
uint8_t _lastEncoderInput;


///////////////////// NETWORK STUFF ///////////////////////

// https://techtutorialsx.com/2021/dddd10/29/esp32-mdns-host-name-resolution/
IPAddress serverIp;

// https://github.com/gilmaimon/ArduinoWebsockets
const char* ssid = ""; //Enter SSID
const char* password = ""; //Enter Password
String _url = "";
const uint16_t websockets_server_port = 80; // Enter server port
const char* websockets_server_path = "/ws"; // Websocket path

using namespace websockets;
WebsocketsClient client;
void onMessageCallback(WebsocketsMessage message);
void onEventsCallback(WebsocketsEvent event, String data);


///////////////////////////// SETUP ///////////////////////////////////////

void setup(void) {
    uint8_t enc_cnt;

    Wire.begin();
    //Reset of all the encoder ìs
    for (enc_cnt = 0; enc_cnt < NUM_ENCODERS; enc_cnt++) {
        RGBEncoder[enc_cnt].reset();
    }

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        // for (;;); // Don't proceed, loop forever
    }

    display.display();

    pinMode(IntPin, INPUT);

    Serial.begin(115200);

    // Initialization of the encoders
    for (enc_cnt = 0; enc_cnt < NUM_ENCODERS; enc_cnt++) {
        RGBEncoder[enc_cnt].begin(
            i2cEncoderLibV2::INT_DATA
            | i2cEncoderLibV2::WRAP_DISABLE
            | i2cEncoderLibV2::DIRE_RIGHT
            | i2cEncoderLibV2::IPUP_ENABLE
            | i2cEncoderLibV2::RMOD_X1
            | i2cEncoderLibV2::RGB_ENCODER);
        RGBEncoder[enc_cnt].id = enc_cnt;
        RGBEncoder[enc_cnt].writeRGBCode(0);
        RGBEncoder[enc_cnt].writeFadeRGB(3); //Fade enabled with 3ms step
        RGBEncoder[enc_cnt].writeAntibouncingPeriod(25); //250ms of debouncing
        RGBEncoder[enc_cnt].writeDoublePushPeriod(0); //Set the double push period to 500ms

        /* Configure the events */
        RGBEncoder[enc_cnt].onChange = encoder_rotated;
        RGBEncoder[enc_cnt].onButtonRelease = encoder_click;
        RGBEncoder[enc_cnt].onMinMax = encoder_thresholds;
        RGBEncoder[enc_cnt].onFadeProcess = encoder_fade;

        /* Enable the I2C Encoder V2 interrupts according to the previus attached callback */
        RGBEncoder[enc_cnt].autoconfigInterrupt();
    }
    
    RGBEncoder[ENC_BRIGHTNESS_ID].writeCounter((int32_t)ENC_BRIGHTNESS_DEFAULT);
    RGBEncoder[ENC_BRIGHTNESS_ID].writeMax((int32_t)ENC_BRIGHTNESS_MAX);
    RGBEncoder[ENC_BRIGHTNESS_ID].writeMin((int32_t)ENC_BRIGHTNESS_MIN);
    RGBEncoder[ENC_BRIGHTNESS_ID].writeStep((int32_t)ENC_BRIGHTNESS_STEP);

    RGBEncoder[ENC_PRESET_ID].writeCounter((int32_t)ENC_PRESET_DEFAULT);
    RGBEncoder[ENC_PRESET_ID].writeMax((int32_t)ENC_PRESET_MAX);
    RGBEncoder[ENC_PRESET_ID].writeMin((int32_t)ENC_PRESET_MIN);
    RGBEncoder[ENC_PRESET_ID].writeStep((int32_t)ENC_PRESET_STEP);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setTextSize(1);

    // delay(1000);
    // testdrawchar();
    WiFi.begin(ssid, password);

    // String myApUrl = String("WIFI:S:") + RB_WIFI_AP + ";T:nopass;P:;;";
    // drawQrCode(myApUrl.c_str(), MESSAGE_CONFIGURE_WIFI);
    // boolean configuredWifi = wifiManager.autoConnect(RB_WIFI_AP);

    // if( !configuredWifi ) {
    //     Serial.println("Wifi Manager Failed!");
    //     display.println("Wifi Manager Failed!");
    //     display.display();
    //     for (;;);
    // }

    Serial.print("Connecting to WiFi...");
    display.println("Connecting WiFi: ");
    display.display();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    Serial.println(" Connected!");
    display.println(" Connected!");
    display.println();
    display.display();

    // WiFi.onEvent(OnWiFiEvent); // TODO

    start_mdns_service();

    Serial.print("Resolving host Lumifera: ");
    display.println("Resolving Lumifera: ");
    display.display();
    while (serverIp.toString() == "0.0.0.0") {
        delay(250);
        serverIp = MDNS.queryHost("Lumifera");
    }	
    Serial.println(serverIp.toString());
    display.println(" " + serverIp.toString());
    display.println();
    display.display();
    _url = "ws://" + serverIp.toString() + ":" + websockets_server_port + websockets_server_path;
    Serial.println(_url);

    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);
    client.connect(_url);
}

/////////////////////// LOOP /////////////////////////////

void loop() {
    uint8_t enc_cnt;

    if (digitalRead(IntPin) == LOW) {
        //Interrupt from the encoders, start to scan the encoder matrix
        for (enc_cnt = 0; enc_cnt < NUM_ENCODERS; enc_cnt++) {
            if (digitalRead(IntPin) == HIGH) { //If the interrupt pin return high, exit from the encoder scan
                break;
            }
            RGBEncoder[enc_cnt].updateStatus();
        }
    }
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
        display.println("Websocket: ");
        display.println(" Connected!");
        display.println();
        display.println("You may begin!");
        display.display();
    }
    else if (event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Websocket: Closed");
        display.println();
        display.display();
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

///////////////////////// WIFI STUFF //////////////////////////////

// void OnWiFiEvent(WiFiEvent_t event)
// {
//   constexpr const char* SGN = "OnWiFiEvent()";
//   String myURL;

//   switch (event) {
 
//     // SYSTEM_EVENT_STA_CONNECTED: ESP32 working as station connected to a WiFi network:
//     case SYSTEM_EVENT_STA_CONNECTED:
//       Serial.printf("%s: %s: SYSTEM_EVENT_STA_CONNECTED: ESP32 Connected to WiFi Network\n", timeToString().c_str(), SGN);
//       myURL = "http://" + WiFi.localIP().toString();
//       // drawQrCode(myURL.c_str(),MESSAGE_OPEN_WEBAPP);
//       break;

//     // SYSTEM_EVENT_AP_START: ESP32 soft AP started;
//     case SYSTEM_EVENT_AP_START:
//       Serial.printf("%s: %s: SYSTEM_EVENT_AP_START: ESP32 soft AP started\n", timeToString().c_str(), SGN);
//       drawQrCode("WIFI:S:Lumifera;T:nopass;P:;;",MESSAGE_JOIN_SOFT_AP);
//       break;

//     // SYSTEM_EVENT_AP_STACONNECTED: station connected to the ESP32 soft AP;
//     case SYSTEM_EVENT_AP_STACONNECTED:
//       Serial.printf("%s: %s: SYSTEM_EVENT_AP_STACONNECTED: Station connected to ESP32 soft AP\n", timeToString().c_str(), SGN);
//       myURL = "http://192.168.4.1"; // hard coded; WiFi.getIP.toString doesnt work.
//       drawQrCode(myURL.c_str(),MESSAGE_OPEN_WEBAPP);
//       break;

//     // SYSTEM_EVENT_AP_STADISCONNECTED: station disconnected to the ESP32 soft AP:
//     case SYSTEM_EVENT_AP_STADISCONNECTED:
//       Serial.printf("%s: %s: SYSTEM_EVENT_AP_STADISCONNECTED: Station disconnected from ESP32 soft AP\n", timeToString().c_str(), SGN);
//       drawQrCode("WIFI:S:Lumifera;T:nopass;P:;;",MESSAGE_JOIN_SOFT_AP);
//       break;

//     default: break;
//   }
// }

///////////////////////////// MISC UTIL ///////////////////////////////////

std::string timeToString()
{
  char myString[20];
  unsigned long nowMillis = millis();
  unsigned int seconds = nowMillis / 1000;
  unsigned int remainder = nowMillis % 1000;
  int days = seconds / 86400;
  seconds %= 86400;
  byte hours = seconds / 3600;
  seconds %= 3600;
  byte minutes = seconds / 60;
  seconds %= 60;
  snprintf(myString, 20, "%02d:%02d:%02d:%02d.%03d", days, hours, minutes, seconds, remainder);
  return std::string(myString);
}

/////////////////////////////////////// ENCODERS ///////////////////////////




