#include <Arduino.h>
#include <i2cEncoderLibV2.h>

void encoder_rotated(i2cEncoderLibV2* obj) {
    // if (obj->readStatus(i2cEncoderLibV2::RINC))
    //     Serial.print("Increment ");
    // else
    //     Serial.print("Decrement ");
    displayParameter(obj->id, obj->readCounterInt());
    obj->writeRGBCode(0x00FF00);
}

void encoder_click(i2cEncoderLibV2* obj) {
    Serial.print("Push: ");
    Serial.println(obj->id);
    obj->writeRGBCode(0x0000FF);
}

void encoder_thresholds(i2cEncoderLibV2* obj) {
    // if (obj->readStatus(i2cEncoderLibV2::RMAX))
    //     Serial.print("Max: ");
    // else
    //     Serial.print("Min: ");
    // Serial.println(obj->id);
    obj->writeRGBCode(0xFF0000);
}

void encoder_fade(i2cEncoderLibV2* obj) {
    obj->writeRGBCode(0x000000);
}

