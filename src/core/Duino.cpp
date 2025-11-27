/**
 * @file Duino.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#if SDK_ARDUINO

#include "Duino.h"
#include <avr/wdt.h>

Stream *Core::log = &Serial;

static long baud = 9600;
static long newBaud = baud;
static uint64_t resetTime = 0;

void Core::begin()
{
    Serial.begin(9600);
    Serial1.begin(9600);
}

void Core::updateBaudRate()
{
    newBaud = Serial.baud();
    if (newBaud != baud) {
        baud = newBaud;
        Serial1.end();
        Serial1.begin(baud);
    }
}

Stream* Core::getSerial(int index)
{
    switch (index)
    {
    case 0:
        return &Serial;
    case 1:
        return &Serial1;
    default:
        break;
    }
    return &Serial;
}

void Core::reset(void)
{
    resetTime = millis() + 1000;
}

void Core::loop(void)
{
    if(resetTime > 0 && millis() > resetTime)
    {
        wdt_enable(WDTO_15MS); // Ativa o watchdog com timeout de 15ms
        while (1); // Força o reset ao esperar o timeout
    }
}

#endif  // USE_ARDUINO
