/**
 * @file STM32Arduino.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#if defined(SDK_STM32DUINO)

#include "STM32Arduino.h"

static long baud = 9600;
static long newBaud = baud;
static uint64_t resetTime = 0;

void Core::begin()
{
    Serial.begin(9600);
}

void Core::updateBaudRate()
{

}

Stream* Core::getSerial(int index)
{
    switch (index)
    {
    case 0:
        return &Serial;
    case 1:
        return NULL;
    default:
        break;
    }
    return &Serial;
}

void Core::reset(void)
{
    resetTime = millis() + 1000;
}


#endif  //!__STM32DUINO__H__