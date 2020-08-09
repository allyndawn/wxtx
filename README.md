# WX-TX

## Overview

You’ve found the GitHub repository for the outdoor unit / transmitter part of my wireless weather station, a.k.a. wx-tx. The outdoor unit / transmitter includes an STM32 Nucleo-F429ZI ARM Cortex M4 development board running FreeRTOS. Temperature, humidity and pressure are measured using a BOSCH BMP280 sensor. Time and location are obtained using a uBlox NEO-6M GPS receiver with a patch antenna. And lastly, it uses a RFM69HCW ISM radio operating at 915 MHz to transmit weather data to the indoor receiver.  The outdoor unit / transmitter transmits the time, location and weather data periodically to the indoor unit / receiver for display and eventual upload to the cloud. All code is written in C, with separate tasks for each peripheral and a “core” task managing messaging between the peripherals.

More detailed information is available in the [WX-TX Project Wiki](https://github.com/allendav/wx-tx/wiki)

## Related Projects

* [WX-RX Project](https://github.com/allendav/wx-rx)
