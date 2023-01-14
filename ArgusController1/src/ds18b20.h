
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// ds18b20.h
// Copyright 2020-2023 Argotronic GmbH
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------

#ifndef _DS18B20_H_
#define _DS18B20_H_

#include <OneWire.h>

#define TEMPERATURE_ERROR 0

class DS18B20 {

public:
    DS18B20()
        : _oneWire(0)
        , _temperature(TEMPERATURE_ERROR)
    {
    }

    bool init(uint8_t pin)
    {
        bool rc = false;

        memset(_addr, 0, sizeof(_addr));
        _oneWire.begin(pin);

        if (_oneWire.search(_addr) == 1) {
            if (OneWire::crc8(_addr, 7) == _addr[7]) {
                if (_addr[0] == 0x10 || _addr[0] == 0x28 || _addr[0] == 0x22) {
                    rc = true;
                    if (_addr[0] == 0x10) {
                        dbgPrint(F("DS18S20 found: "));
                    } else if (_addr[0] == 0x28) {
                        dbgPrint(F("DS18B20 found: "));
                    } else {
                        dbgPrint(F(" DS1820 found: "));
                    }
                }
            } else {
                dbgPrint(F("Unknown Device: "));
            }
            for (uint8_t i = 0; i < 8; i++) {
                dbgHex(_addr[i]);
            }
        } else {
            dbgPrint(F("No sensor"));
        }
        dbgPrintln("");

        _oneWire.reset_search();

        if (!rc) {
            memset(_addr, 0, sizeof(_addr));
        }
        return rc;
    }

    void start()
    {
        _oneWire.reset();
        _oneWire.select(_addr);
        _oneWire.write(0x44);    // start conversion
        // for parasite power devices, e.g. DS18S20P, use ow->write(0x44, 1);
    }

    void read()
    {
        _temperature = TEMPERATURE_ERROR;

        _oneWire.reset();
        _oneWire.select(_addr);
        _oneWire.write(0xBE);    // read scratchpad

        uint8_t data[9];
        for (uint8_t i = 0; i < 9; i++) {
            data[i] = _oneWire.read();
        }

        if (OneWire::crc8(data, 8) == data[8]) {
            int16_t raw = ((int16_t)data[1] << 8) | data[0];
            if (_addr[0] == 0x10) {
                raw = raw << 3;    // 9 bit resolution default
                if (data[7] == 0x10) {
                    // count remain: 12 bit resolution
                    raw = (raw & 0xFFF0) + 12 - data[6];
                }
            } else {
                byte cfg = (data[4] & 0x60);
                // at lower resolution, low bits are undefined
                if (cfg == 0x00) {
                    raw = raw & ~7;    // 9 bit
                } else if (cfg == 0x20) {
                    raw = raw & ~3;    // 10 bit
                } else if (cfg == 0x40) {
                    raw = raw & ~1;    // 11 bit
                }
            }
            _temperature = (raw * 10) / 16;
        } else {
            dbgPrintln(F("CRC Error"));
        }
    }

    int16_t temperature() { return _temperature; }

private:
    ::OneWire _oneWire;
    int16_t   _temperature;
    uint8_t   _addr[8];
};

#endif