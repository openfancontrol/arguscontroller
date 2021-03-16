
//---------------------------------------------------------
// Argus Controller Hardware
// main module
// Copyright 2020-2021 Argotronic UG (haftungsbeschraenkt)
// Version: 1.02
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//
// for Arduino Nano or Arduino Uno
// Arduino Nano driver (CH340 version): http://www.wch.cn/downloads/CH341SER_EXE.html
//---------------------------------------------------------

// start user configuration
//=========================================================
#define DEVICE_ID 1    // Argus Monitor can manage up to 4 different Argus Controller devices

#define TEMPSENSOR_COUNT 4

#define PIN_TEMPSENSOR_1 A0
#define PIN_TEMPSENSOR_2 A1
#define PIN_TEMPSENSOR_3 A2
#define PIN_TEMPSENSOR_4 A3

#define FAN_COUNT 2

//#define DEBUG_OUTPUT
//#define FAKE_SENSORS
// #define PIN_LED 13 // Arduino Nano built-in LED, free to use
//=========================================================
// end user configuration

// clang-format off
#include "src/interface.h"
// clang-format on
#include "src/amcom.h"
#include "src/debug.h"
#include "src/ds18b20.h"
#include "src/fanctrl.h"
#include <EEPROM.h>

AMCOM<DEVICE_ID, TEMPSENSOR_COUNT, FAN_COUNT> amCom;
DS18B20                                       tempSensor[TEMPSENSOR_COUNT];
bool                                          tempSensorPresent[TEMPSENSOR_COUNT];
const uint8_t                                 owPin[TEMPSENSOR_COUNT] = { PIN_TEMPSENSOR_1, PIN_TEMPSENSOR_2, PIN_TEMPSENSOR_3, PIN_TEMPSENSOR_4 };
FANCTRL                                       fanctrl;
uint8_t                                       buffer[20];

//---------------------------------------------------------
void setup()
{
    delay(200);

    // flash LED on App start
#ifdef PIN_LED
    pinMode(PIN_LED, OUTPUT);
    for (uint8_t i = 0; i < 2; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(25);
        digitalWrite(PIN_LED, LOW);
        delay(200);
    }
#endif

    Serial.begin(57600);

    dbgPrintln("");
    for (uint8_t i = 0; i < TEMPSENSOR_COUNT; i++) {
        dbgPrint("Channel ");
        dbgDec(i + 1);
        dbgPrint(": ");
        tempSensorPresent[i] = tempSensor[i].init(owPin[i]);
    }

    fanctrl.init(FAN_COUNT);
}

//---------------------------------------------------------
void loop()
{
    for (uint8_t i = 0; i < TEMPSENSOR_COUNT; i++) {
        if (tempSensorPresent[i]) {    // process only avalilable sensors
            tempSensor[i].start();
        }
    }

    // wait 1.1sec DS18B20 max. conversion time (resolution dependent)
    for (uint8_t i = 0; i < 11; i++) {
        amCom.delay(100);
        processCommands();
    }

    dbgPrintln("");
    for (uint8_t i = 0; i < TEMPSENSOR_COUNT; i++) {
        if (tempSensorPresent[i]) {    // process only avalilable sensors
            tempSensor[i].read();
            dbgPrint("Channel ");
            dbgDec(i + 1);
            dbgPrint(" temperature: ");
            dbgDecln(tempSensor[i].temperature());
        }
    }

    // wait 1sec while receiving
    for (uint8_t i = 0; i < 10; i++) {
        amCom.delay(100);
        processCommands();
    }

    fanctrl.update();
}

//---------------------------------------------------------
void processCommands()
{
    if (amCom.queueCount() > 0) {
        uint32_t qdata = amCom.queuePop();
        uint8_t  cmd   = qdata & 0xFF;
        uint8_t  channel;
        switch (cmd) {
        case AMAC_CMD::CmdGetTemp:
            buffer[0] = cmd;
            buffer[1] = TEMPSENSOR_COUNT;
            for (uint8_t i = 0; i < TEMPSENSOR_COUNT; i++) {
                int16_t temperature;
#ifdef FAKE_SENSORS
                temperature = 305 + 10 * i;    // 30.5C + i
#else
                temperature = tempSensor[i].temperature();
#endif
                buffer[2 + i * 2] = temperature >> 8;
                buffer[3 + i * 2] = temperature & 0xFF;
            }
            amCom.send(buffer, 2 + 2 * TEMPSENSOR_COUNT);
            break;
        case AMAC_CMD::CmdGetFanRpm:
            buffer[0] = cmd;
            buffer[1] = FAN_COUNT;
            for (uint8_t i = 0; i < FAN_COUNT; i++) {
                uint16_t rpm;
#ifdef FAKE_SENSORS
                rpm = 2000 + 100 * i;
#else
                rpm         = fanctrl.getRpm(i);
#endif
                buffer[2 + i * 2] = rpm >> 8;
                buffer[3 + i * 2] = rpm & 0xFF;
            }
            amCom.send(buffer, 2 + 2 * FAN_COUNT);
            break;
        case AMAC_CMD::CmdGetFanPwm:
            channel   = (qdata >> 8) & 0xFF;
            buffer[0] = cmd;
            buffer[1] = channel;
            buffer[2] = fanctrl.getPwm(channel);
            amCom.send(buffer, 3);
            break;
        case AMAC_CMD::CmdSetFanPwm: {
            channel     = (qdata >> 8) & 0xFF;
            uint8_t pwm = (qdata >> 16) & 0xFF;
            if (fanctrl.setPwm(channel, pwm)) {
                buffer[0] = cmd;    // ok code
            } else {
                buffer[0] = 0xFF;    // error code
            }
            amCom.send(buffer, 1);
            break;
        }
        case AMAC_CMD::CmdEEReadByte: {
            uint16_t eeAddr = (qdata >> 8) & 0xFFFF;
            buffer[0]       = cmd;
            buffer[1]       = 1;
            buffer[2]       = EEPROM.read(eeAddr);
            amCom.send(buffer, 3);
            break;
        }
        case AMAC_CMD::CmdEEWriteByte: {
            uint16_t eeAddr = (qdata >> 8) & 0xFFFF;
            uint8_t  value  = (qdata >> 24) & 0xFF;
            if (value != EEPROM.read(eeAddr)) {
                EEPROM.write(eeAddr, value);
                delay(20);    // wait for EE value to be written
            }
            buffer[0] = cmd;    // ok code (if needed, do an additional verify here)
            amCom.send(buffer, 1);
            break;
        }
        default:
            break;
        }
    }
}
