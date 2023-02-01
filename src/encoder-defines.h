#define NUM_ENCODERS 2 // Number of encoders

// Brightness
#define ENC_BRIGHTNESS_ID 0
#define ENC_BRIGHTNESS_ADDRESS 0x24
#define ENC_BRIGHTNESS_NAME "brightness"
#define ENC_BRIGHTNESS_MIN 0
#define ENC_BRIGHTNESS_MAX 254
#define ENC_BRIGHTNESS_STEP 5
#define ENC_BRIGHTNESS_DEFAULT 80
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


const String encoderNames[NUM_ENCODERS] = { ENC_BRIGHTNESS_NAME, 
                                            ENC_PRESET_NAME };
const bool encoderConfirm[NUM_ENCODERS] = { ENC_BRIGHTNESS_CONFIRM, 
                                            ENC_PRESET_CONFIRM };
