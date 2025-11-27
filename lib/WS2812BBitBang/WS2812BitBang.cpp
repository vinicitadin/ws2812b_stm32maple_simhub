#include "WS2812BitBang.h"

#ifdef SDK_MAPLE
    #include <libmaple/gpio.h>
    #include <libmaple/dma.h>
#endif

WS2812_BitBang::WS2812_BitBang(uint16_t numLeds, uint8_t pin) {
    _numLeds = numLeds;
    _pin = pin;
    _pixels = new uint8_t[numLeds * 3];
    memset(_pixels, 0, numLeds * 3);
}

WS2812_BitBang::~WS2812_BitBang() {
    delete[] _pixels;
}

void WS2812_BitBang::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    
#ifdef SDK_MAPLE
    gpio_dev *dev = digitalPinToPort(_pin);
    _pinMask = digitalPinToBitMask(_pin);
    _portSet = &(dev->regs->BSRR);
    _portClear = &(dev->regs->BRR);
#endif
}

void WS2812_BitBang::clear() {
    memset(_pixels, 0, _numLeds * 3);
}

void WS2812_BitBang::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n < _numLeds) {
        uint16_t idx = n * 3;
        _pixels[idx] = g;
        _pixels[idx + 1] = r;
        _pixels[idx + 2] = b;
    }
}

void WS2812_BitBang::setPixelColor(uint16_t n, uint32_t color) {
    setPixelColor(n, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

uint32_t WS2812_BitBang::Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void WS2812_BitBang::show() {
#ifdef SDK_MAPLE
    noInterrupts();
    
    uint8_t *ptr = _pixels;
    uint16_t count = _numLeds * 3;
    
    while (count--) {
        sendByte(*ptr++);
    }
    
    interrupts();
    delayMicroseconds(80);
#endif
}

void WS2812_BitBang::sendByte(uint8_t b) {
#ifdef SDK_MAPLE
    volatile uint32_t *set = _portSet;
    volatile uint32_t *clr = _portClear;
    uint32_t mask = _pinMask;
    
    for (uint8_t i = 0; i < 8; i++) {
        if (b & 0x80) {
            *set = mask;
            asm volatile(
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                ::: "memory"
            );
            *clr = mask;
            asm volatile(
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t"
                ::: "memory"
            );
        } else {
            *set = mask;
            asm volatile(
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t"
                ::: "memory"
            );
            *clr = mask;
            asm volatile(
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                ::: "memory"
            );
        }
        b <<= 1;
    }
#endif
}