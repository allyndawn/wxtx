# WX-TX (Outdoor Unit / Transmitter)

## Overview

You’ve found the GitHub repository for the outdoor unit / transmitter part of my wireless weather station, a.k.a. WX-TX.

A STM32 Nucleo-F429ZI ARM Cortex M4 development board running FreeRTOS serves as the heart of the outdoor unit. A BOSCH BMP280 sensor provides temperature, humidity and pressure measurements. A uBlox NEO-6M GPS receiver with a patch antenna provides time and location. An RFM69HCW ISM radio operating at 915 MHz transmits time, location and weather data periodically to the indoor unit / receiver for display and eventual upload to the cloud.

All code is written in C, with separate modules and tasks for each peripheral and a “core” task managing messaging between the peripherals.

More detailed information is available in the [WX-TX Project Wiki](https://github.com/allendav/wx-tx/wiki)

## Related Projects

* [WX-RX (Indoor Unit / Receiver) Project](https://github.com/allendav/wx-rx)
