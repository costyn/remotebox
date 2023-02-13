#include <Arduino.h>
#include <i2cEncoderLibV2.h>

void encoder_rotated(i2cEncoderLibV2* obj) {
    encoderColorFeedback(obj, ROTATE);
    // if (obj->readStatus(i2cEncoderLibV2::RINC))
    //     Serial.print("Increment ");
    // else
    //     Serial.print("Decrement ");
    displayParameter(obj->id, obj->readCounterInt());
    sendParameter(obj->id, obj->readCounterInt(), false);
}

void encoder_click(i2cEncoderLibV2* obj) {
    encoderColorFeedback(obj, CLICK);
    sendParameter(obj->id, obj->readCounterInt(), true);
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

    