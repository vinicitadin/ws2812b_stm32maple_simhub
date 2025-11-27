/**
 * @file ILed.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-11-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __ILED__H__
#define __ILED__H__

#include <Arduino.h>
#include <WS2812B.h>

// Define timing mais lento para compatibilidade
#define FASTLED_INTERRUPT_RETRY_COUNT 1

class ILed : public WS2812B
{
private:
    uint16_t count;
public:
    ILed(uint16_t count) : WS2812B(count) { this->count = count; }

    void begin()
    { 
        WS2812B::begin();
        WS2812B::show();
    }

    void setBrightness(uint8_t brightness) { WS2812B::setBrightness(brightness); }
    void setPixelColor(uint8_t id, uint8_t r, uint8_t g, uint8_t b) { WS2812B::setPixelColor(id, r, g, b); }
    void show() { WS2812B::show(); }

    uint16_t getCount() { return count; }
};

#endif  //!__ILED__H__