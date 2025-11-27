/**
 * @file PrsCommSimhub.cpp
 * @author your name (you@domain.com)
 * @brief Módulo de comunicação com o Simhub
 * @version 0.1
 * @date 2024-07-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <WString.h>

#include "CommSimhub.h"

String command;

CommSimhub::CommSimhub(ILed *leds, Stream *serialPc, Stream *serialDisplay, uint8_t displayType)
{
    this->serialPc = serialPc;
    this->serialDisplay = serialDisplay;
    this->displayType = displayType;
    this->leds = leds;
}

void CommSimhub::begin()
{

}

void CommSimhub::loop()
{
    while (serialPc->available())
    {
        writeToComputer();
        char c = serialPc->read();

        if (messageend < 6)
        {
            if (c == (char)0xFF)
            {
                messageend++;
            }
            else
            {
                messageend = 0;
            }
        }

        if (messageend >= 3 && c != (char)(0xff))
        {
            command += c;
            while (command.length() < 5)
            {
                writeToComputer();
                while (!serialPc->available())
                {
                }
                c = serialPc->read();
                command += c;
            }

            // Get protocol version
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)proto
            if (command == F("proto"))
            {
                serialPc->println(F(PROTOCOLVERSION));
            }

            // Get device name
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dname
            else if (command == F("dname"))
            {
                serialPc->println(F(TARGET_NAME));
            }

            // Get brand name
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dname
            else if (command == F("bname"))
            {
                serialPc->println(F(TARGET_BRAND));
            }

            // Get firmware version
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dname
            else if (command == F("fwver"))
            {
                serialPc->println(F(TARGET_FIRMWARE_VERSION));
            }

            // Get device picture
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dname
            else if (command == F("dpict"))
            {
                serialPc->println(F(TARGET_PICTURE_URL));
            }

            // Get device auto detect state
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dname
            else if (command == F("ddete"))
            {
                serialPc->println((int)DEVICE_AUTODETECT_ALLOWED);
            }

            // Get device type
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)dtype
            else if (command == F("dtype"))
            {
                serialPc->println((int)TARGET_SIMHUB_DEVICE_TYPE);
            }

            // Get upload protection informations
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)ulock
            else if (command == F("ulock"))
            {
                serialPc->print(ENABLE_UPLOAD_PROTECTION);
                serialPc->print(",");
                serialPc->println(UPLOAD_AVAILABLE_DELAY);
            }

            // *** SERIAL NUMBER  ***

            // Get serial number
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)snumb
            else if (command == F("snumb"))
            {
                serialPc->println("TESTE");
            }

            // Reset serial number
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)snumb
            else if (command == F("rnumb"))
            {
                serialPc->println("TESTE");
            }

            // *** LEDS ***

            // Get leds count
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)ledsc
            else if (command == F("ledsc"))
            {
                serialPc->println(leds->getCount());
            }

            // Get leds Layout
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)ledsl
            else if (command == F("ledsl"))
            {
                serialPc->println(F(LEDS_LAYOUT));
            }

            // Get default buttons profile colors
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)butdc
            else if (command == F("butdc"))
            {
                serialPc->println(F(DEFAULT_BUTTONS_COLORS));
            }

            // Send leds data (in binary) terminated by (0xFF)(0xFE)(0xFD)
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)sleds(RL1)(GL1)(BL1)(RL2)(GL2)(BL2) .... (0xFF)(0xFE)(0xFD)
            else if (command == F("sleds"))
            {
                readLeds(serialPc);
            }

            // *** MATRIX ***

            // Get 8x8 matrix count
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)matxc
            else if (command == F("matxc"))
            {
                serialPc->println(MATRIX_ENABLED > 0 ? 1 : 0);
            }

            // Set 8x8 matrix content
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)sleds(RL1)(GL1)(BL1)(RL2)(GL2)(BL2) .... (0xFF)(0xFE)(0xFD)
            else if (command == F("smatx"))
            {
                // readMatrix();
            }

            // *** FANS ***

            // Get fans count
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)fansc
            else if (command == F("fansc"))
            {
                serialPc->println(0);
            }

            // Set fans values (16bits)
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)sfans(FAN1 MSB)(FAN1 LSB)(FAN2 MSB)(FAN2 LSB)(FAN3 MSB)(FAN3 LSB) .... (0xFF)(0xFE)(0xFD)
            else if (command == F("sfans"))
            {
                // readFans();
            }

            // Vendor message
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)vendo
            else if (command == F("vendo"))
            {
                // int size = waitAndReadOneByte(serialPc);
                // size = ((int)waitAndReadOneByte(serialPc) << 8) | size;
                // if (commPrs != nullptr)
                // {
                //     uint8_t data[size];
                //     serialPc->readBytes(data, size);
                //     commPrs->processData(data, size);
                // }
            }

            // Unlock upload
            // (0xFF)(0xFF)(0xFF)(0xFF)(0xFF)(0xFF)unloc
            else if (command == F("unloc"))
            {
                uploadUnlocked = false;
            }

            else if(serialDisplay != nullptr)
            {
                serialDisplay->print(command);
            }
            command = "";
            messageend = 0;
        }
        else if(serialDisplay != nullptr)
        {
            serialDisplay->write(c);
        }
    }
    writeToComputer();
}

void CommSimhub::writeToComputer()
{
    if(serialDisplay == nullptr) return;
    while (serialDisplay->available())
    {
        char c = (char)serialDisplay->read();
        serialPc->write(c);
    }
}

void CommSimhub::readLeds(Stream *serial)
{
    uint8_t r, g, b;
    int ledsCount = leds->getCount();

    for (int i = 0; i < ledsCount; i++)
    {
        r = waitAndReadOneByte(serial);
        g = waitAndReadOneByte(serial);
        b = waitAndReadOneByte(serial);
        leds->setPixelColor(i, r, g, b);
    }
    leds->show();
    delayMicroseconds(300);
}

int CommSimhub::waitAndReadOneByte(Stream *stream)
{
    while (!stream->available())
    {
    }
    return stream->read();
}
