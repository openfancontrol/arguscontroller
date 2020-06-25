
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// amcom.h
// Copyright 2020 Argotronic UG (haftungsbeschraenkt)
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------

// clang-format off
/*-----------------------------------------------------------------------------
Protocol

Command             Argus Monitor request               Argus Controller answer
ProbeDevice         AA 02 01 crc8                       C5 <byteCnt> 01 <DEVICE_ID> <TEMP_COUNT> <FAN_COUNT> crc8
GetTemp             AA 02 20 crc8                       C5 <byteCnt> 20 <TEMP_COUNT> temp0_H temp0_L temp1_H temp1_L temp2_H temp2_L temp3_H temp3_L crc8
GetFanRpm           AA 02 30 crc8                       C5 <byteCnt> 30 <FAN_COUNT> rpm0_H rpm0_L rpm1_H rpm1_L crc8
GetFanPwm           AA 03 31 <channel> crc8             C5 <byteCnt> 31 <channel> <pwm> crc8
SetFanPwm           AA 04 32 <channel> <pwm> crc8       C5 <byteCnt> 32 crc8            answer byte2: 32 = ok, FF = error

Data formats
  temperature: int16_t, scaled by 10
  rpm: uint16_t
  pwm: uint8_t [0..100 %]

Communication parameters
57600 Baud, 8N1

Only for the ProbeDevice command, Argus Monitor expects the answer from the device within 200msec.

If the 'Argus Controller hardware support' option in Settings/Stability is enabled,
Argus Monitor will probe the specified COM-Ports for Argus Controller devices.
It will use the first 4 devices (if specified and available) as additional HW Monitor sources within the application.

-----------------------------------------------------------------------------*/
// clang-format on

#ifndef _AMCOM_H_

#include <util/crc16.h>
#include "debug.h"
#include "queue.h"

enum AMCMD { CmdProbeDevice = 0x01, CmdGetTemp = 0x20, CmdGetFanRpm = 0x30, CmdGetFanPwm = 0x31, CmdSetFanPwm = 0x32 };

template <uint8_t DEVID, uint8_t TEMPCNT, uint8_t FANCNT> class AMCOM {

public:
    AMCOM()
        : timeStartMsg(0)
        , receiveState(0)
        , receiveLength(0)
        , queue(10)
    {
        memset(rawBuffer, 0, sizeof(rawBuffer));
        memset(receiveBuffer, 0, sizeof(receiveBuffer));
    }

    void delay(unsigned long delay_ms)
    {
        unsigned long timeStamp = millis();
        while ((millis() - timeStamp) < delay_ms) {
            receive();
        }
    }

    bool send(uint8_t* buffer, uint8_t length)
    {
        if (length > (sizeof(rawBuffer) - 6)) {
            return false;
        }
        uint8_t len      = 0;
        rawBuffer[len++] = 0xC5;
        rawBuffer[len++] = length + 1;    // bytes to come including crc
        for (uint8_t i = 0; i < length; i++) {
            rawBuffer[len++] = buffer[i];
        }
        uint8_t crc8     = _crc8(rawBuffer, len);
        rawBuffer[len++] = crc8;
        for (uint8_t i = 0; i < len; i++) {
            Serial.write(rawBuffer[i]);
        }
        delay(50);    // decouple consecutive send messages
        memset(rawBuffer, 0, sizeof(rawBuffer));
        return true;
    }

    int queueCount() { return queue.count(); }

    uint32_t queuePop() { return queue.pop(); }

private:
    unsigned long   timeStartMsg;
    uint8_t         rawBuffer[32];
    uint8_t         receiveBuffer[20];
    uint8_t         receiveState;
    uint8_t         receiveLength;
    uint8_t*        receivePtr;
    Queue<uint32_t> queue;

    void receive()
    {
        while (Serial.available()) {
            int data = Serial.read();

            switch (receiveState) {
            case 0:
                if (data == 0xAA) {
                    memset(receiveBuffer, 0, sizeof(receiveBuffer));
                    receivePtr    = receiveBuffer;
                    *receivePtr++ = data;
                    receiveState  = 1;
                    timeStartMsg  = millis();
                }
                break;
            case 1:
                if ((data >= 2) && (data <= 4)) {
                    *receivePtr++ = data;
                    receiveLength = data;
                    receiveState  = 2;
                } else {
                    receiveState = 0;
                }
                break;
            case 2:
                *receivePtr++ = data;
                receiveLength--;
                if (receiveLength == 0) {
                    uint8_t  len  = receiveBuffer[1] + 1;    // all message bytes without crc8 byte
                    uint8_t  crc8 = _crc8(receiveBuffer, len);
                    uint32_t qc;
                    if (crc8 == data) {
                        uint8_t cmd = receiveBuffer[2];
                        switch (cmd) {
                        case CmdProbeDevice:    // answer CmdProbeDevice at once (200msec timeout in Argus Monitor on Argus Controller init)
                            uint8_t b[4];
                            b[0] = cmd;
                            b[1] = DEVID;
                            b[2] = TEMPCNT;
                            b[3] = FANCNT;
                            send(b, 4);
                            break;
                        case CmdGetTemp:
                        case CmdGetFanRpm:
                            queue.push((uint32_t)cmd);
                            break;
                        case CmdGetFanPwm:
                            qc = cmd | (((uint32_t)receiveBuffer[3]) << 8);    // cmd, channel
                            queue.push(qc);
                            break;
                        case CmdSetFanPwm:
                            qc = cmd | (((uint32_t)receiveBuffer[3]) << 8) | (((uint32_t)receiveBuffer[4]) << 16);    // cmd, channel, pwm value
                            queue.push(qc);
                            break;
                        default:
                            break;
                        }
                    } else {
                        dbgPrintln("CRC Error");
                    }
                    receiveState = 0;
                }
                break;
            default:
                break;
            }
        }

        // reset message receive state machine on incomplete messages with a 250msec timeout
        if ((receiveState != 0) && ((millis() - timeStartMsg) > 250)) {
            receiveState = 0;
            dbgPrintln("receiveState reset");
        }
    }

    uint8_t _crc8(uint8_t* data, uint8_t len)
    {
        uint8_t crc = 0;
        while (len--) {
            crc = _crc_ibutton_update(crc, *data++);
        }
        return crc;
    }
};

#endif
