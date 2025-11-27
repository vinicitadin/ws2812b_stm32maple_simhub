/**
 * @file Maple.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#if defined(SDK_MAPLE)

#include "core/Maple.h"
#include "libmaple/rcc.h"
#include "constants/constants.h"

USBHID Core::HID;
Stream *Core::log;
USBCompositeSerial Core::compositeSerial;

static uint64_t resetTime = 0;

void Core::begin()
{
    afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY);
    USBComposite.setManufacturerString(TARGET_BRAND);
    USBComposite.setProductString(TARGET_NAME);
    USBComposite.setVendorId(PRS_VENDOR_ID);
    USBComposite.setProductId(TARGET_PID);

    USBComposite.clear();

    compositeSerial.begin();    

    log = &compositeSerial;
}

void Core::updateBaudRate()
{
    
}

Stream* Core::getSerial(int index)
{
    if(index >= 1) return NULL;
    return &Core::compositeSerial;
}

void Core::reset(void)
{
    resetTime = millis() + 1000;
}

void Core::loop(void)
{
    if(resetTime > 0 && millis() > resetTime)
    {
        nvic_sys_reset();
    }
}
#endif  //SDK_MAPLE