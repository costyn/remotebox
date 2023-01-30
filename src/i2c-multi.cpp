#include <Arduino.h>
#include "i2c-multi.h"
#include <Wire.h>
#include <i2cEncoderLibV2.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h> 
#include <ArduinoWebsockets.h>
// #include <Fonts/FreeSans12pt7b.h>
#include <mdns-service.h>
#include <encoder-defines.h>
#include <encoder.h>

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


#define NUM_ENCODERS 2 // Number of encoders

i2cEncoderLibV2 RGBEncoder[NUM_ENCODERS] = { i2cEncoderLibV2(ENC_0_ADDRESS),
                                             i2cEncoderLibV2(ENC_1_ADDRESS) };
const String encoderNames[NUM_ENCODERS] = { ENC_0_NAME, 
                                            ENC_1_NAME };
const bool encoderConfirm[NUM_ENCODERS] = { ENC_0_CONFIRM, 
                                            ENC_1_CONFIRM };


const int IntPin = A3; // INT pins, change according to your board

const char* ssid = "***REMOVED***"; //Enter SSID
const char* password = "***REMOVED***"; //Enter Password
const char* websockets_server_host = "192.168.178.71"; //Enter server adress
const uint16_t websockets_server_port = 8080; // Enter server port

using namespace websockets;

WebsocketsClient client;

//Class initialization with the I2C addresses
// i2cEncoderLibV2 RGBEncoder[ENCODER_N] = { i2cEncoderLibV2(0x40),
//                                           i2cEncoderLibV2(0x20), i2cEncoderLibV2(0x60), i2cEncoderLibV2(0x10),
//                                           i2cEncoderLibV2(0x50), i2cEncoderLibV2(0x30), i2cEncoderLibV2(0x70),
//                                           i2cEncoderLibV2(0x04), i2cEncoderLibV2(0x44),
// };

uint8_t encoder_status, i;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

    // start_mdns_service();

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
    
    RGBEncoder[ENC_0_ID].writeCounter((int32_t)ENC_0_DEFAULT);
    RGBEncoder[ENC_0_ID].writeMax((int32_t)ENC_0_MAX);
    RGBEncoder[ENC_0_ID].writeMin((int32_t)ENC_0_MIN);
    RGBEncoder[ENC_0_ID].writeStep((int32_t)ENC_0_STEP);

    RGBEncoder[ENC_1_ID].writeCounter((int32_t)ENC_1_DEFAULT);
    RGBEncoder[ENC_1_ID].writeMax((int32_t)ENC_1_MAX);
    RGBEncoder[ENC_1_ID].writeMin((int32_t)ENC_1_MIN);
    RGBEncoder[ENC_1_ID].writeStep((int32_t)ENC_1_STEP);

    // delay(1000);
    // testdrawchar();
}

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
}

void displayParameter(int id, int value) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    // display.setFont(&FreeMonoBoldOblique12pt7b);
    display.setTextSize(2);
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