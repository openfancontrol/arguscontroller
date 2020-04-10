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

todo


## Lizenz

**Creative Commons BY-SA**<br>
Give Credit, ShareAlike

<a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.
