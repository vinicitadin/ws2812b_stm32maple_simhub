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
#include <WS2812BitBang.h>

class ILed : public WS2812_BitBang
{
private:
    uint16_t count;
public:
    ILed(uint16_t count, uint8_t pin) : WS2812_BitBang(count, pin) { this->count = count; }

    void begin()
    { 
        WS2812_BitBang::begin();
        WS2812_BitBang::clear();
        WS2812_BitBang::show();
    }

    // void setBrightness(uint8_t brightness) { WS2812_BitBang::setBrightness(brightness); }
    void setPixelColor(uint8_t id, uint8_t r, uint8_t g, uint8_t b) { WS2812_BitBang::setPixelColor(id, r, g, b); }
    void show() { WS2812_BitBang::show(); }

    uint16_t getCount() { return count; }
};

#endif  //!__ILED__H__