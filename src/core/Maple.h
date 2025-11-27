/**
 * @file Maple.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __MAPLE__H__
#define __MAPLE__H__

#if SDK_MAPLE

#include <USBComposite.h>
#include <Arduino.h>
#include "usb_serial.h"

#ifndef TARGET_SERIAL_COUNT
#define TARGET_SERIAL_COUNT 2
#endif

class Core
{
private:
public:
    static USBHID HID;
    static USBCompositeSerial compositeSerial;
    static Stream *log;
    static void begin();
    static void updateBaudRate();
    static Stream* getSerial(int index);
    static void reset(void);
    static void loop(void);
};
#endif  //SDK_MAPLE

#endif  //!__MAPLE__H__
