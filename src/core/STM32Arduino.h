/**
 * @file STM32Arduino.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __STM32DUINO__H__
#define __STM32DUINO__H__

#if SDK_STM32DUINO

#include <Arduino.h>

class Core
{
private:
    
public:
    static Stream *log;
    static void begin();
    static void updateBaudRate();
    static Stream* getSerial(int index);
    static void reset(void);
    static void loop(void);
};

#endif

#endif  //!__STM32DUINO__H__
