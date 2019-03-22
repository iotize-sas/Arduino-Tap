# Arduino-Tap

The tap library allows an Arduino board to be connected to a TapNLink using S3P protocol. 

The main characteristics of the TapNLink solution are:

 - quick way to wirelessly connect any embedded system to a mobile device,
 - various variants such as NFC + BLE, NFC + BLE, NFC + BLE + LoRa,
 - no coding required, just Tap configuration,
 - mobile apps for Android, iOS and Win10 are automatically generated from IoTize Studio.
 - STM32 target firmware can be updated

 The possible protocols between the TapNLink (Tap) and the target (Arduino) are: 
  - **SWD** (debug) for boards with a Cortex-M processor (DUE, STM32 nucleo),
  - **S3P** for all processors (only two digital IOs are required). S3P is an SPI-like protocol (one clock and one synchronous data), plus one interrupt to initiate the communication. The only difficulty is that the pulse for the IRQ is generated on the SPI clock. Because the volume of the data to transfer is low, this SPI is managed by software. The master is always the Arduino (but the IRQ is generated by the Tap), and the Tap manages the SPI with a DMA channel to support the fastest  processors.  

This library is dedicated to the **S3P** protocol. It is not used for SWD protocol (that runs without modifying the firmware). 

There are two submodes with S3P and only the 'indexed' mode has been implemented in this library. 

## Hardware

To use this library, you need: 

- TapNLink module: A 'Primer Tap' (evaluation kit) is perfect as long as you don't use TapNLink in production. 
Tested with TapNLink NFC + BLE, HW version 1.10 and FW version 1.43. This kit is [available at Digikey](https://www.digikey.com/product-detail/en/iotize/TNL-PRIMER-NB/2087-TNL-PRIMER-NB-ND/9923057). 

- Arduino board: This library has been tested with Arduino Uno, Due and Mega. The example can be used without any change to these boards. 
Other boards require you to modify the interrupt handler (minor task).

- 4 wires to connect the boards. For a 5V Arduino processor, a 1K resistor must be serially inserted on both clock and data signals (the Tap is powered by 3V). 

## Getting Started

Specific help is available at:  [extras/howto.md](extras/howto.md)


## Documentation

Please visit the IoTize web site, and more precisely the [documentation center](http://docs.iotize.com/).
