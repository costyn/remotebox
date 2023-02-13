#include <Arduino.h>
#include "i2c-multi.h"
#include "mdns-service.h"
#include "encoder-defines.h"
#include "encoder.h"

/////////// TODO
///// WifiManager
///// TaskScheduler

/*In this example there are 9 I2C Encoder V2 boards with the RGB LED connected in a matrix 3x3
  There is also the Arduino Serial KeyPad Shield attached.

  Every time of one encoder is moved it's value is showed on the display
  In this example the data type are float

  Connections with Arduino UNO:
  - -> GND
  + -> 5V
  SDA -> A4
  SCL -> A5
  INT -> A3

    For the STEMMA QT cables, we follow the Qwiic convention:

    Black for GND
    Red for V+
    Blue for SDA
    Yellow for SCL
*/

///////////////// ENCODER STUFF /////////////////////////

i2cEncoderLibV2 RGBEncoder[NUM_ENCODERS] = { i2cEncoderLibV2(ENC_BRIGHTNESS_ADDRESS),
                                             i2cEncoderLibV2(ENC_PRESET_ADDRESS) };

//Class initialization with the I2C addresses
// i2cEncoderLibV2 RGBEncoder[ENCODER_N] = { i2cEncoderLibV2(0x40),
//                                           i2cEncoderLibV2(0x20), i2cEncoderLibV2(0x60), i2cEncoderLibV2(0x10),
//                                           i2cEncoderLibV2(0x50), i2cEncoderLibV2(0x30), i2cEncoderLibV2(0x70),
//                                           i2cEncoderLibV2(0x04), i2cEncoderLibV2(0x44),
// };


const int IntPin = A3; // INT pins, change according to your board
uint8_t _lastEncoderInput;

/////////////////// DISPLAY STUFF /////////////////////

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

QRCode qrCode;

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

    WiFi.onEvent(OnWiFiEvent);

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

////////////////// OLED DISPLAY //////////////////////////////

void displayParameter(int id, int value) {
    if (!client.available()) {
        return;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setTextSize(2);
    display.dim(1);
    String label = encoderNames[id];
    drawCentreString(label, 0);

    if( encoderConfirm[id]) {
        display.setTextSize(4);
        drawCentreString(String(value), 22);
        display.setTextSize(1);
        drawCentreString("Click to confirm", 56);
    } else {
        display.setTextSize(4);
        drawCentreString(String(value), 28);
    }
    display.display();

    Serial.print(encoderNames[id]);
    Serial.print(": ");
    Serial.println(value);
}

void drawCentreString(const String &buf, int y)
{
    int16_t x1, y1, x;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    // display.setCursor(x - w / 2, y);
    display.setCursor((SCREEN_WIDTH - w) / 2, y);

    display.println(buf);
}

void testdrawchar(void) {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}

void drawQrCode(const char* qrStr, const char* lines[]) {
	uint8_t qrcodeData[qrcode_getBufferSize(3)];
	qrcode_initText(&qrCode, qrcodeData, 3, ECC_LOW, qrStr);
 
  display.clearDisplay();
  Serial.println("Updating OLED display");

  // Text starting point
  int cursor_start_y = 10;
  int cursor_start_x = 4;
  int font_height = 12;

	// QR Code Starting Point
  int offset_x = 62;
  int offset_y = 3;

  for (int y = 0; y < qrCode.size; y++) {
      for (int x = 0; x < qrCode.size; x++) {
          int newX = offset_x + (x * 2);
          int newY = offset_y + (y * 2);

          if (qrcode_getModule(&qrCode, x, y)) {
							display.fillRect( newX, newY, 2, 2, 0);
          }
          else {
							display.fillRect( newX, newY, 2, 2, 1);
          }
      }
  }
  display.setTextColor(1,0);
  for (int i = 0; i < 4; i++) {
    display.setCursor(cursor_start_x,cursor_start_y+font_height*i);
    display.println(lines[i]);
  }
  display.display();
}

//////////////////////// WEBSOCKETS ////////////////////////////////

void sendParameter(int id, int value, boolean onClick) {
    if (!client.available()) {
        return;
    }
    if (id == ENC_BRIGHTNESS_ID || onClick) {
        String json = "{" + encoderNames[id] + ": " + value + "}\n";
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

void OnWiFiEvent(WiFiEvent_t event)
{
  constexpr const char* SGN = "OnWiFiEvent()";
  String myURL;

  switch (event) {
 
    // SYSTEM_EVENT_STA_CONNECTED: ESP32 working as station connected to a WiFi network:
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.printf("%s: %s: SYSTEM_EVENT_STA_CONNECTED: ESP32 Connected to WiFi Network\n", timeToString().c_str(), SGN);
      myURL = "http://" + WiFi.localIP().toString();
      // drawQrCode(myURL.c_str(),MESSAGE_OPEN_WEBAPP);
      break;

    // SYSTEM_EVENT_AP_START: ESP32 soft AP started;
    case SYSTEM_EVENT_AP_START:
      Serial.printf("%s: %s: SYSTEM_EVENT_AP_START: ESP32 soft AP started\n", timeToString().c_str(), SGN);
      drawQrCode("WIFI:S:Lumifera;T:nopass;P:;;",MESSAGE_JOIN_SOFT_AP);
      break;

    // SYSTEM_EVENT_AP_STACONNECTED: station connected to the ESP32 soft AP;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.printf("%s: %s: SYSTEM_EVENT_AP_STACONNECTED: Station connected to ESP32 soft AP\n", timeToString().c_str(), SGN);
      myURL = "http://192.168.4.1"; // hard coded; WiFi.getIP.toString doesnt work.
      drawQrCode(myURL.c_str(),MESSAGE_OPEN_WEBAPP);
      break;

    // SYSTEM_EVENT_AP_STADISCONNECTED: station disconnected to the ESP32 soft AP:
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.printf("%s: %s: SYSTEM_EVENT_AP_STADISCONNECTED: Station disconnected from ESP32 soft AP\n", timeToString().c_str(), SGN);
      drawQrCode("WIFI:S:Lumifera;T:nopass;P:;;",MESSAGE_JOIN_SOFT_AP);
      break;

    default: break;
  }
}

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

void setPreset(int presetIndex){
     RGBEncoder[ENC_PRESET_ID].writeCounter((int32_t)presetIndex);   
}

void setBrightness(int brightness){
     RGBEncoder[ENC_BRIGHTNESS_ID].writeCounter((int32_t)brightness);   
}


void encoderColorFeedback(i2cEncoderLibV2* obj, EncoderEvent event) {
    _lastEncoderInput = obj->id; // not really the right place to put it
    if (client.available()) {
        if (event == ROTATE) {
            obj->writeRGBCode(0x00FF00);
        }
        if (event == CLICK) {
            obj->writeRGBCode(0x0000FF);
        }
        if (event == LIMIT) {
            obj->writeRGBCode(0xFF0000);
        }
    }
    else {
        obj->writeRGBCode(0x4c00b0); // purple
    }
}