
//---------------------------------------------------------
// Argus Controller (Open Hardware)
// interface.h
// Copyright 2020-2023 Argotronic GmbH
//
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
// You are free to Share & Adapt under the following terms:
// Give Credit, ShareAlike
//---------------------------------------------------------


// clang-format off
/*-----------------------------------------------------------------------------
Protocol

Command             Argus Monitor request                       Argus Controller answer
ProbeDevice         AA 02 01 crc8                               C5 <byteCnt> 01 <DEVICE_ID> <TEMP_COUNT> <FAN_COUNT> crc8
GetTemp             AA 02 20 crc8                               C5 <byteCnt> 20 <TEMP_COUNT> temp0_H temp0_L temp1_H temp1_L temp2_H temp2_L temp3_H temp3_L crc8
GetFanRpm           AA 02 30 crc8                               C5 <byteCnt> 30 <FAN_COUNT> rpm0_H rpm0_L rpm1_H rpm1_L crc8
GetFanPwm           AA 03 31 <channel> crc8                     C5 <byteCnt> 31 <channel> <pwm> crc8
SetFanPwm           AA 04 32 <channel> <pwm> crc8               C5 <byteCnt> 32/FF crc8                         # answer byte2: 32 = ok, FF = error
EEReadByte          AA 04 40 <addrH> <addrL> crc8               C5 <byteCnt> 40 <VALUE_COUNT> <val> crc8
EEWriteByte         AA 05 41 <addrH> <addrL> <value> crc8       C5 <byteCnt> 41/FF crc8                         # answer byte2: 41 = ok, FF = error

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


#ifndef _INTERFACE_H_


enum AMAC_CMD {
    CmdUndefined   = 0x00,
    CmdProbeDevice = 0x01,
    CmdGetTemp     = 0x20,
    CmdGetFanRpm   = 0x30,
    CmdGetFanPwm   = 0x31,
    CmdSetFanPwm   = 0x32,
    CmdEEReadByte  = 0x40,
    CmdEEWriteByte = 0x41,
    CmdError       = 0xFF
};

#define EEADDR_PWM_POWERON_0 0x28
#define EEADDR_PWM_POWERON_1 0x29
#define EEADDR_PWM_POWERON_2 0x2A
#define EEADDR_PWM_POWERON_3 0x2B


#endif
