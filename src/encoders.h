#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>
#include <i2cEncoderLibV2.h>
#include <ArduinoWebsockets.h>
#include "i2c-multi.h"

#define NUM_ENCODERS 2 // Number of encoders

// Brightness
#define ENC_BRIGHTNESS_ID 0
#define ENC_BRIGHTNESS_ADDRESS 0x24
#define ENC_BRIGHTNESS_NAME "brightness"
#define ENC_BRIGHTNESS_MIN 0
#define ENC_BRIGHTNESS_MAX 254
#define ENC_BRIGHTNESS_STEP 5
#define ENC_BRIGHTNESS_DEFAULT 150
#define ENC_BRIGHTNESS_CONFIRM false

// Preset
#define ENC_PRESET_ID 1
#define ENC_PRESET_ADDRESS 0x20
#define ENC_PRESET_NAME "preset"
#define ENC_PRESET_MIN 0
#define ENC_PRESET_MAX 7
#define ENC_PRESET_STEP 1
#define ENC_PRESET_DEFAULT 0
#define ENC_PRESET_CONFIRM true


const String encoderNames[NUM_ENCODERS] PROGMEM = { ENC_BRIGHTNESS_NAME, 
                                                    ENC_PRESET_NAME };
const bool encoderConfirm[NUM_ENCODERS] PROGMEM = { ENC_BRIGHTNESS_CONFIRM, 
                                                    ENC_PRESET_CONFIRM };

//Class initialization with the I2C addresses
// i2cEncoderLibV2 RGBEncoder[ENCODER_N] = { i2cEncoderLibV2(0x40),
//                                           i2cEncoderLibV2(0x20), i2cEncoderLibV2(0x60), i2cEncoderLibV2(0x10),
//                                           i2cEncoderLibV2(0x50), i2cEncoderLibV2(0x30), i2cEncoderLibV2(0x70),
//                                           i2cEncoderLibV2(0x04), i2cEncoderLibV2(0x44),
// };


static i2cEncoderLibV2 RGBEncoder[NUM_ENCODERS] = { i2cEncoderLibV2(ENC_BRIGHTNESS_ADDRESS),
                                             i2cEncoderLibV2(ENC_PRESET_ADDRESS) };

const int IntPin = A3; // INT pins, change according to your board

void encoderSetup();
void readEncoders();

void encoder_rotated(i2cEncoderLibV2* obj);
void encoder_click(i2cEncoderLibV2* obj);
void encoder_thresholds(i2cEncoderLibV2* obj);
void encoder_fade(i2cEncoderLibV2* obj);
void encoderColorFeedback(i2cEncoderLibV2* obj, EncoderEvent event);

///////////// EXTERNS /////////////////////
void displayParameter(String encoderName, int value, boolean confirmWithClick);
extern websockets::WebsocketsClient client;

#endif