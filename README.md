# Arduino-Tap

The tap library allows to connect any Arduino board to a TapNLink module using S3P protocol. 
The main characteristics of the TapNLink solution are:

 - a quick way to wireless connect any embedded system to a mobile device,
 - various variants such as NFC + BLE, NFC + BLE, NFC + BLE + LoRa,
 - no coding, just configuring the Tap,
 - Mobile Apps for Android, iOS and Win10 are automatically generated from IoTize Studio.
 - For STM32 you can even update the target firmware.

 Note that there are two main possible protocols between the TapNLink (tap) and the target (Arduino): 
  - SWD (debug) protocol for the boards with  Cortex-M processor (DUE, STM32 nucleo),
  - or S3P for all processors (only two digital IOs are required). 

This library is not used for SWD protocol (that runs without modifying the firmware) and it is dedicated to the S3P protocol. 

There are two submodes with S3P and only the 'indexed' mode has been implemented in this library. 

## Hardware

To use this library, you need: 

### TapNLink module

You need a TapNLink module. A 'Primer Tap' (evaluation kit) is perfect as long as you don't use TapNLink in production. 
Tested with TapNLink NFC + BLE, HW version 1.10 and FW version 1.43.

### Arduino boards

This library has been tested with Arduino Uno, Due and Mega. The example can be used without any change with rhses three boards. 
It works also for any other board but you will have to modify the interrupt handler (minor task).

## Getting Started

A specific help is available at:  [extras/howto.md](extras/howto.md)


## Documentation

Please visit the IoTize web site, and more precisely the [documentation center](http://docs.iotize.com/).
