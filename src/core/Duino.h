/**
 * @file Duino.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __ARDUINO__H__
#define __ARDUINO__H__

#if SDK_ARDUINO

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

#endif  //!__PRS_ARDUINO__H__
