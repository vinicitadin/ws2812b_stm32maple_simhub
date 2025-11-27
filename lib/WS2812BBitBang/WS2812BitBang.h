#ifndef WS2812_BITBANG_H
#define WS2812_BITBANG_H

#include <Arduino.h>

class WS2812_BitBang {
public:
    WS2812_BitBang(uint16_t numLeds, uint8_t pin);
    ~WS2812_BitBang();
    
    void begin();
    void show();
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void setPixelColor(uint16_t n, uint32_t color);
    void clear();
    
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
    
private:
    uint16_t _numLeds;
    uint8_t _pin;
    uint8_t *_pixels;
    
    volatile uint32_t *_portSet;
    volatile uint32_t *_portClear;
    uint32_t _pinMask;
    
    void sendByte(uint8_t b);
    void sendBit(bool bitVal);
};

#endif