#include <Arduino.h>
#include "comm/CommSimhub.h"

#if SDK_STM32DUINO
#include "core/STM32Arduino.h"
#elif SDK_MAPLE
#include "core/Maple.h"
#elif SDK_ARDUINO
#include "core/Duino.h"
#endif

#define LEDS_COUNT 82

ILed leds = ILed(LEDS_COUNT);

CommSimhub commSimhub(&leds, Core::getSerial(0), Core::getSerial(1), 0);

void setup()
{
    rcc_clk_enable(RCC_AFIO);
    afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);
    afio_remap(AFIO_REMAP_SPI1); 
    
    Core::begin();
    leds.begin();
}

void loop()
{
    commSimhub.loop();
}