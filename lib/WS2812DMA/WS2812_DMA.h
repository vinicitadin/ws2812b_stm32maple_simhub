#ifndef __WS2812_DMA__H__
#define __WS2812_DMA__H__

#include <Arduino.h>

class WS2812_DMA
{
private:
    uint8_t pin;
    uint8_t count;
    uint8_t bytes;
    uint16_t *ws2812Buffer;
    uint16_t bufferSize;
public:
    WS2812_DMA(uint8_t pin, uint8_t count);
    void setPixelColor(uint16_t led, uint8_t r, uint8_t g, uint8_t b);
    void begin();
    void show();
    void configurePinMapping(uint8_t pin);
};

#endif  //!__WS2812_DMA__H__