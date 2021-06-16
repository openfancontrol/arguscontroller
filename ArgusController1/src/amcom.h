
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// amcom.h
// Copyright 2020-2021 Argotronic UG (haftungsbeschraenkt)
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------

#ifndef _AMCOM_H_

#include "queue.h"
#include <util/crc16.h>

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
                if ((data >= 2) && (data <= 5)) {    // known commands have 2..5 bytes length codes
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
                            // cmd, channel
                            qc = cmd | (((uint32_t)receiveBuffer[3]) << 8);
                            queue.push(qc);
                            break;
                        case CmdSetFanPwm:
                        case CmdEEReadByte:
                            // CmdSetFanPwm:  cmd, channel, pwm value
                            // CmdEEReadByte: cmd, addrH, addrL
                            qc = cmd | (((uint32_t)receiveBuffer[3]) << 8) | (((uint32_t)receiveBuffer[4]) << 16);
                            queue.push(qc);
                            break;
                        case CmdEEWriteByte:
                            // cmd, addrH, addrL, value
                            qc = cmd | (((uint32_t)receiveBuffer[3]) << 8) | (((uint32_t)receiveBuffer[4]) << 16)
                                 | (((uint32_t)receiveBuffer[5]) << 24);
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
