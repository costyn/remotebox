#include <Arduino.h>
#include <TaskScheduler.h>
#include <JC_Button.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HttpClient.h>


#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

#define DEFAULT_BRIGHTNESS 80
#define DEFAULT_LEDMODE 12

#define BUTTON_PIN 23
#define TASK_CHECK_BUTTON_PRESS_INTERVAL 100 // in milliseconds

#define POT_BRIGHTNESS_PIN 18
#define POT_SPEED_PIN 19
#define POT_HIGH_PIN 22
#define LED_INDICATOR_PIN 1

#define NUMROUTINES 5




const char* ssid = "Supernova";
const char* password =  "yesyesyes";

Scheduler _runner;

int _ledMode = DEFAULT_LEDMODE;
uint8_t _speed;
uint8_t _brightness;
Button nextPatternButton(BUTTON_PIN);

HTTPClient httpClient;
String serverName = "http://192.168.4.1/update";

void checkPots();
void checkButtonPress();
void autoAdvanceLedMode();


Task taskCheckButtonPress(TASK_CHECK_BUTTON_PRESS_INTERVAL, TASK_FOREVER, &checkButtonPress);
Task taskAutoAdvanceLedMode(15 * TASK_SECOND, TASK_FOREVER, &autoAdvanceLedMode);


void setup() {
    _runner.init();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Connecting to: ");
        Serial.println(ssid);
        delay(1000);
    }


    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);

    pinMode(POT_HIGH_PIN, OUTPUT);    // 3v source
    digitalWrite(POT_HIGH_PIN, HIGH); // 3v source
    pinMode(POT_BRIGHTNESS_PIN, INPUT);
    pinMode(POT_SPEED_PIN, INPUT);
    pinMode(LED_INDICATOR_PIN, OUTPUT);    // 3v source

    nextPatternButton.begin();
    _runner.addTask(taskCheckButtonPress);
    taskCheckButtonPress.enable();
}

void loop() {
    _runner.execute();
}


void sendHTTP() {
    String url = serverName + "?speed=" + _speed;
           url = url + "&brightness=" + _brightness;
           url = url + "&pattern=" + _ledMode;
    httpClient.begin(url.c_str());
    httpClient.GET();
}


void nextLedMode()
{
  _ledMode++;
  if (_ledMode == NUMROUTINES)
  {
    _ledMode = 0;
  }
  sendHTTP();
}

void prevPattern()
{
  if (_ledMode == 0)
  {
    _ledMode = NUMROUTINES;
  }
  _ledMode--;
  sendHTTP();
}

void autoAdvanceLedMode()
{
  nextLedMode();
}

#define LONGPRESS 1000

void checkButtonPress()
{
  nextPatternButton.read();

  if (nextPatternButton.pressedFor(LONGPRESS))
  {
    DEBUG_PRINTLN("LONGPRESS detected");
    if (taskAutoAdvanceLedMode.isEnabled())
    {
      taskAutoAdvanceLedMode.disable();
      DEBUG_PRINTLN("autoAdvance: DISABLED");
    }
    else
    {
      taskAutoAdvanceLedMode.enable();
      DEBUG_PRINTLN("autoAdvance: ENABLED");
    }
  } 
  else if (nextPatternButton.wasReleased())
  {
    DEBUG_PRINTLN("SHORTpress detected");
    nextLedMode();
    digitalWrite(LED_INDICATOR_PIN,HIGH);
    digitalWrite(LED_INDICATOR_PIN,LOW);
  }

  checkPots();
}

void checkPots()
{
  static uint16_t lastBrightness;
  static uint16_t lastSpeed;

  static uint32_t Pot1Old;
  static uint32_t Pot1EMA_S = 0; //initialization of EMA S
  float Pot1EMA_A = 0.20;        //initialization of EMA alpha
  uint32_t Pot1Current = analogRead(POT_BRIGHTNESS_PIN);
  Pot1EMA_S = (Pot1EMA_A * Pot1Current) + ((1 - Pot1EMA_A) * Pot1EMA_S);

  uint16_t calculatedBrightness;

  if (Pot1EMA_S != Pot1Old) {
    Pot1Old = Pot1EMA_S;
    DEBUG_PRINT("Pot1EMA_S: ");
    DEBUG_PRINT(Pot1EMA_S);
    calculatedBrightness = 255 - map(Pot1EMA_S, 0, 800, 0, 253);
    if (calculatedBrightness != lastBrightness)
    {
      lastBrightness = calculatedBrightness;
      _brightness = calculatedBrightness;
      sendHTTP();
      DEBUG_PRINT("\tBrightness: ");
      DEBUG_PRINTLN(lastBrightness);
    }
  }

  static uint32_t Pot2Old;
  static uint32_t Pot2EMA_S = 0; //initialization of EMA S
  float Pot2EMA_A = 0.20;        //initialization of EMA alpha
  uint32_t Pot2Current = analogRead(POT_SPEED_PIN);
  Pot2EMA_S = (Pot2EMA_A * Pot2Current) + ((1 - Pot2EMA_A) * Pot2EMA_S);

  uint16_t calculatedSpeed;

  if (Pot2EMA_S != Pot2Old)
  {
    Pot2Old = Pot2EMA_S;
    calculatedSpeed = 256 - map(Pot2EMA_S, 0, 800, 3, 255);
    if (calculatedSpeed != lastSpeed)
    {
      lastSpeed = calculatedSpeed;
      _speed = lastSpeed;
    sendHTTP();
      DEBUG_PRINT("Speed: ");
      DEBUG_PRINTLN(lastSpeed);
    }
  }
}