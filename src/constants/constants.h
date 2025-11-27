#ifndef STANDARD_ENUMS
#define STANDARD_ENUMS

// Fans controller
#define FANS_NOFAN 0
#define FANS_25KHZPWM 1
#define FANS_CUSTOM 99

// Matrix layouts
#define MATRIX_LEFTTORIGHT 0
#define MATRIX_SERPENTINE 1
#define MATRIX_REVERSESERPENTINE 2

// Device types
#define DEVICETYPE_UNSPECIFIED 0
#define DEVICETYPE_LEDS 1
#define DEVICETYPE_LEDS_PLUS_VOCORE 2
#define DEVICETYPE_LEDS_PLUS_USBD480 3
#define DEVICETYPE_LEDS_PLUS_MONITOR 4
#define DEVICETYPE_MATRIX 11
#define DEVICETYPE_MATRIX_PLUS_LEDS 12
#define DEVICETYPE_FANS 21

#define DEVICE_AUTODETECT_ALLOWED 1
#define ENABLE_UPLOAD_PROTECTION 0
#define UPLOAD_AVAILABLE_DELAY 15000

#define PROTOCOLVERSION "SIMHUB_1.2"

#define TARGET_FIRMWARE_VERSION "0.1.0.3"
#define TARGET_PICTURE_URL "Test"
#define TARGET_NAME "STM32F4 Test Device"
#define TARGET_BRAND "PRS"
#define TARGET_SIMHUB_DEVICE_TYPE 0


// Example "L0,L1,L2,B0,B0,B1,B1,B2,B2,B2"
// A led or a button can be used multiple times if needed (IE if the 4 first LEDs are tied to the first button : B0,B0,B0,B0 ...
// If nothing is specified all leds will be used as a telemetry leds.
#define LEDS_LAYOUT ""

// Default buttons colors at connection
// Example "#FFFFFF,#FFFFFF"
#define DEFAULT_BUTTONS_COLORS ""

//-------------------------
// ------- 8x8 WS2812B RGB Matrix Settings
//-------------------------
// Enable matrix support ? Set to 0 to disable.
#define MATRIX_ENABLED 0

#define PRS_VENDOR_ID 0x16c0
#define TARGET_PID 0x3103

#endif
