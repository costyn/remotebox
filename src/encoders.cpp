#include "my_globals.h"
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


void encoderSetup() {
    uint8_t enc_cnt;

    pinMode(IntPin, INPUT);
    
    //Reset of all the encoder ìs
    for (enc_cnt = 0; enc_cnt < NUM_ENCODERS; enc_cnt++) {
        RGBEncoder[enc_cnt].reset();
    }

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
}

void readEncoders() {
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