#include "encoders.h"

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
*/


void encoder_rotated(i2cEncoderLibV2* obj) {
    encoderColorFeedback(obj, ROTATE);
    // if (obj->readStatus(i2cEncoderLibV2::RINC))
    //     Serial.print("Increment ");
    // else
    //     Serial.print("Decrement ");
    displayParameter(encoderNames[obj->id], obj->readCounterInt(), encoderConfirm[obj->id]);
    sendParameter(encoderNames[obj->id], obj->readCounterInt(), !encoderConfirm[obj->id]);
}

void encoder_click(i2cEncoderLibV2* obj) {
    encoderColorFeedback(obj, CLICK);
    sendParameter(encoderNames[obj->id], obj->readCounterInt(), encoderConfirm[obj->id]);
    Serial.print("Push: ");
    Serial.println(obj->id);
        
}

void encoder_thresholds(i2cEncoderLibV2* obj) {
    encoderColorFeedback(obj, LIMIT);
    // if (obj->readStatus(i2cEncoderLibV2::RMAX))
    //     Serial.print("Max: ");
    // else
    //     Serial.print("Min: ");
    // Serial.println(obj->id);
}

void encoder_fade(i2cEncoderLibV2* obj) {
    obj->writeRGBCode(0x000000);
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

void setPreset(int presetIndex){
     RGBEncoder[ENC_PRESET_ID].writeCounter((int32_t)presetIndex);   
}

void setBrightness(int brightness){
     RGBEncoder[ENC_BRIGHTNESS_ID].writeCounter((int32_t)brightness);   
}