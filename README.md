# Argus Controller

<p align="center"><img src="Images/ArgusController_140.png?raw=true"/></p>

- Argus Monitor supports Open Hardware circuits to use additional custom temperature sensors and fan control channels within the program.
- We call these devices Argus Controller.
- At least Argus Monitor version 5.0.4 is required for these.


## Description

We have made an [example](ArduinoNanoExample1) to demonstrate such a device with the help of the very common Arduino Nano or Arduino Uno platform.<br>
The example demonstrates the creation and set-up of temperature channels and fan control channels.<br>
We show here a hardware solution for the popular Dallas DS18B20 temperature sensors and for connecting 4-pin pwm controlled fans.<br>
With additional circuitry and software changes, you could control 3-pin voltage controlled fans also or use different  temperature sensors.<br><br>
You can adapt the hardware to your needs, built it around a completely different microcontroller, change the number of  temperature and fan control channels and so on.<br>
Each hardware device can have up to 6 temperature channels and 6 fan control channels.<br>
Argus Monitor will detect and use any such device as long as the serial communication protocol (see below) is respected.<br>

[:us: Detailed HW/SW description (English)](https://help.argusmonitor.com/ArgusController.html)

[:de: Detaillierte HW/SW Beschreibung (Deutsch)](https://hilfe.argusmonitor.com/ArgusController.html)

![pic](Images/ArgusControllerNano1.jpg)

![pic](Images/ArgusControllerNano2.jpg)


## Communication protocol

| Command    | Argus Monitor request         | Argus Controller answer |
|---|---|---|
|ProbeDevice | AA 02 01 crc8                 | C5 [byteCnt] 01 [DEVICE_ID] [TEMP_COUNT] [FAN_COUNT] crc8 |
|GetTemp     | AA 02 20 crc8                 | C5 [byteCnt] 20 [TEMP_COUNT] temp0_H temp0_L temp1_H temp1_L temp2_H temp2_L temp3_H temp3_L crc8 |
|GetFanRpm   | AA 02 30 crc8                 | C5 [byteCnt] 30 [FAN_COUNT] rpm0_H rpm0_L rpm1_H rpm1_L crc8 |
|GetFanPwm   | AA 03 31 [channel] crc8       | C5 [byteCnt] 31 [channel] [pwm] crc8 |
|SetFanPwm   | AA 04 32 [channel] [pwm] crc8 | C5 [byteCnt] 32 crc8   (answer byte2: 32 = ok, FF = error) |

- All numbers are hex.
- The second bytes is always the count of remaining bytes in this message, beginning with the next (third) byte.
- Data formats
  - temperature: int16_t, scaled by 10
  - rpm: uint16_t
  - pwm: uint8_t [0..100 %]
- Communication parameters
  - 57600 Baud, 8N1
- Only for the ProbeDevice command, Argus Monitor expects the answer from the device within 200msec.
- If the 'Argus Controller hardware support' option in Settings/Stability is enabled, Argus Monitor will probe the specified COM-Ports for Argus Controller devices.
It will use the first 4 devices (if specified and available) as additional HW Monitor sources within the application.

- CRC8 calculation
```
uint8_t crc8(uint8_t crc, uint8_t data)
{
    crc = crc ^ data;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x01)
            crc = (crc >> 1) ^ 0x8C;
        else
            crc >>= 1;
    }
    return crc;
}
```


## Lizenz

**Creative Commons BY-SA**<br>
Give Credit, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
