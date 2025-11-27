/**
 * @file PrsCommSimhub.h
 * @author your name (you@domain.com)
 * @brief Módulo de comunicação com o Simhub
 * @version 0.1
 * @date 2024-07-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __PRSCOMMSIMHUB__H__
#define __PRSCOMMSIMHUB__H__

#include <Stream.h>

#include "constants/constants.h"
#include "led/ILed.h"

class CommSimhub
{
private:
    Stream *serialPc;
    Stream *serialDisplay;
    uint8_t displayType;
    ILed *leds;
    int messageend;
    bool uploadUnlocked;
    void readLeds(Stream *serial);
    int waitAndReadOneByte(Stream *serial);
public:
    CommSimhub(ILed *leds, Stream *serialPc = nullptr, Stream *serialDisplay = nullptr, uint8_t displayType = 0);
    void begin();
    void loop();
    void writeToComputer();
};

#endif  //!__PRSCOMMSIMHUB__H__
