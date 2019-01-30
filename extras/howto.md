# How to IoTize your Arduino board

This application note shows how to add wireless connectivity to an Arduino application by plugging a TapNLink. 

Only a few minutes are required to iotize a Cortex-M based embedded application. But with most of Arduino boards, Integration is made easy thanks to the library concept. 

## Supported Arduino hardware

Arduino is a family based on different architecture. 
Practically, TapNLink supports any microcontroller and any architecture.  But depending on the board, the way to add a TapNLink module could slightly vary.
The main criteria are the voltage of the I/O pins, and the core. 
We have tested here on 3 popular boards: 



| Model | Processor | IOs Voltage |     
|:-----:|:---------:|:------------|
| Uno	| AVR (ATmega328)	| 5V |
| Due	| Cortex-M (ATSAM3X8E)	| 3V |
| Mega2560	| AVR (ATmega2560)	| 5V |

### Core dependency

We had to adapt slightly the interface: 
 - For the  Cortex-M based boards, we can use either the S3P protocol or the debug protocol (SWD). SWD is managed by the hardware of the core and is very simple to be use. It offers: 
        - the ability to start immediatly, without modifying the firmware,
        - various advanced features such as the update of the target (Arduino) firmware from a mobile.

S3P provides a better security level, but security is not always the most important criteria for a Proof of Concept. Therefore, SWD would be preferred when available (e.g. for the Cortex-M based processors). 

 - for the other processors  (and potentially for some Cortex-M based as well), the TAP library must be added. This document describes how to use this library. 
 
### I/O voltage issue

For processors with 5V digital pins, we have to adapt the voltage levels. Several solutions are possible, but simply insert a resistor for each digital signal. 
 
Whatever the protocol (SWD/S3P), we need 4 wires to make the Arduino board communicating with TapNLink:

|  Type   |  Name    | Description |     
|:-------:|:--------:|:------------|
| Power	  |  Gnd     |   Ground          |
| Power	  |  Vcc3_3  |   MUST BE 3V or 3.3V |
| Digital |  CLK     | Must be an interrupt (or could be SWD-CLK for Cortex-M devices) |
| Digital |  IO      |   Clk and IO must be adapted (resistor) for 5V rocessors  |
	

To summarize, two wires for the power supply (GND et Vcc3.3) provided by the Arduino to TapNLink and two digital signals: clock and data. 

Optionally, we can add the reset signal if we wish to be able to reset the Arduino board from the TapNLink module. 

WARNING: With a 5V board, connect the 3.3V pin. **DO NOT CONNECT 5V**, it will destroy your Tap!

## Let's connect the boards

### The signals at the TapNLink side

For both SWD and S3P, the 4 signals are available on two connectors:
 - a small 10-contact, 1.27mm (ARM compatible) dual row connector,
 - or a bigger 5-contact, 2.54mm single row connector
If you purchase a Primer, connect the provided 5-wires cable as follows:     

<img src="res/tapconnect.png" alt="Tap connectors" style="max-width: 300px; border: 1px solid gray;">

The pinout is as follows: 

|  Type   |  Signal | 5-pin header   | wire   | 10-pin header (ARM) |      
|:-------:|:-------:|:--------------:|:------:|:-------------------:|
| Power	  |  Vcc3.3 | 1              | Red    | 1                   |
| Power	  |  Gnd    | 2              | Black  | 3,5,9               |
| Digital |  CLK    | 4              | Pink   | 4                   |
| Digital |  IO     | 3              | Blue   | 2                   |
| Digital |  RST    | 5              | Purple | 10                  |



### Connecting a 5V arduino Uno or MEGA2560

Again, take care not connecting the Tap to the 5V power. But we also need to adapt the voltage of the digital inputs: applying directly a 5V 'push-pull' output to the TapNLInk inputs could damage the processor of the TapNLink. The simplest solution consists in inserting a 1 k-ohm (2200 ohm) between the TapNLink and the Arduino for both the Clock and the IO signals. Such a resistor  will limit the current without degrading too much the signals.   

The schematic below shows the connection between Arduino-Uno and TapNLink:

 <img src="res/Arduino-to-Tap.png" alt="Wire connection to TapNLink" style="max-width: 300px; border: 1px solid gray;">

#### Clock signal
It must be connected to an interrupt input of the processor. Not all pins are eligible and the following table summarizes which pins are usable for interrupts:

| Board | Digital Pins Usable For Interrupts | Status |
|:-------:|:-------------------:|:---------:|
| Uno, Nano, Mini, other 328-based | 2,3 | Tested |
| Uno WiFi Rev.2 | all digital pins | Not tested |
| Mega, Mega2560, MegaADK | 2, 3, 18, 19, 20, 21 | Tested |
| Micro, Leonardo, other 32u4-based | 0, 1, 2, 3, 7 | Not tested |
| Zero | all digital pins, except 4 | not tested |
| MKR Family boards | 0, 1, 4, 5, 6, 7, 8, 9, A1, A2 | Not tested |
| Due | all digital pins | Tested |
| 101 | all digital pins | Not tested |


Thus, if we start with Arduino Uno, only pins 2 and 3 can be used for CLK. 
IO signal can be connected to any digital I/O. 

In our example, CLK is connected to pin 3 for the Arduino UNO board and IO (data) is connected to pin 5. This can be modified directly when calling the Init() function:
        myTap.Init(3,5); // clk = 3 and data = 5

### Connecting TapNLink to the the Arduino DUE (or any Cortex-M board)

#### Arduino DUE with SWD 
When the SWD protocol is used, the library is useless and you just need to connect TapNLink to the debug port (SWD / JTAG).
If you own a TapNLink primer, you have to add a 2x5 male header(pitch 1.27mm)  on your TapNLink:

<img src="res/add_swd_connector.png" alt="Missing ARM connector." style="max-width: 300px; border: 1px solid gray;">

You can then link the two board with a simple ribbon (10 wires, 0.635mm pitch):

<img src="res/swd_ribbon_cable.png" alt="Linking the boards." style="max-width: 300px; border: 1px solid gray;">


#### Aduino DUE with S3P
Of course, it is also possible to use the Due board with S3P. In this case, we can select any pair of digital signals available on the connectors. In the example, we selected pin 16 for CLK and 17 for data (IO). Don't forget to connect Vcc (3.3V) and Gnd. 
Note that the signals of the 5 pin connector (single row) are also on the 10 pin ARM connector (double row). You can use either on or the other (the 5-pin is often easier to connect for Arduino boards).  

### Other Cortex-M boards with SWD
The debug port is not always available on all boards. Often the boards provided by the silicon vendors (Nucleo from ST, ...) are equipped with an embedded debugger connected to the debug port. In such a case, we could encounter several situations:
  - either the debug is port is not available (no connector)... SWD must be discarded and S3P used.
  - or the debug port exists, but the debugger cannot be disabled. Again, you have to go with S3P.
  - sometinmes, a connector exists and the debugger stays in Hi-Z as long as its USB port is left unconnected. 
  - or jumpers allow to disconnect the embedded debugger... 
Thus, many cases, and you will have to analyse the schematic (and sometimes to try) to check whether an embedded debugger is or is not a problem. 
On the DUE board, the debug board is just connected to the named "jtag connector". We can use it without restriction. 

### Other processors
It is not difficult to adapt the library (Tap.cpp file) to any processor. The only change to be considered is the clearing of the interrupt flag (see Tap::ConfigureIOs() in tap.cpp).
This file has just to be edited to adapt this part. 
Note that clearing the IRQ flags is mandatory: on most processors, toggling the clock signal that has been used to trigger the interrupt keeps the irq flag active during the interrupt processing. At the return from the handler, the flag would be still active and we would launch for ever the interrupt handler. 

## Relocating the output directory 
To configure from IoTize Studio, we need the elf file containing thevlist of the global symbols (output of the linker). By default, the Arduino IDE generates this file into a temporary directory but we can specify a more suitable location, easily accessible. 

You need to open the  ‘Preferences.txt’ file. You will find it by clicking on: 
        File | Preferences | More preferences can be edited... 

When the 'preferences.txt' file is opened, close the Arduino IDE because it would  overwrite our new preferences.txt file and we would loose our changes. 
Add the line:   
        ‘build.path=…’ 
and specify after '=' an 'output' subfolder of your sketchbook directory. For example, in my case, my sketchbook directory is E:\Documents\Arduino:  

 <img src="res/preferences.png" alt="Specify output directory for Studio" style="max-width: 300px; border: 1px solid gray;">

Save this file. You can now reopen Arduino IDE and the new generated will be saved into this 'output' subfolder. 

## Start with IoTize Studio
IoTize is a free software available on the iotize.com web site:

        http://docs.iotize.com/Downloads/

Please refer to the documentation available on the same website: 

        http://docs.iotize.com/UserManuals/IotizeStudio/

### Create a new project (.iotz)
We assume when using the tap library that you create your IoTize Studio projects into:

        {sketchbook_folder}/TapNLink/

Any other location should work but, when selecting S3P, the tap.cpp finds the S3P_conf.h file using a relative reference. In case you place your iotz files anywhere else, you will have to modify this path.  

The tree should look like: 
        {sketchbook_folder}/
                project1/
                        project1.ino
                project2/
                        project2.ino
                ...
                projectN/
                        projectN.ino
                libraries/
                        tap/
                                examples/
                                        tapdemo.ino
                                src/
                                        tap.cpp
                                        tap.h
                                library.properties
                                keywords.txt
                                readme.md
                        other_libs/
                                ...
                output/
                        core/
                        libraries/
                        preproc/
                        sketch/
                        project1.ino.elf
                        project1.ino.map
                        project2.ino.elf
                        project2.ino.map
                        ...
                TapNLink/
                        project1.iotz
                        project2.iotz
                        S3P_conf.h
                                

#### Main settings

#### Adding variables

## Modify your existing Arduino file (.ino)
Once the TAP library installed, you have to include it in your project (e.g. to add #include "tap.h" at the top of your .ino file.
You need them to initialise the tap handler. This is done by addng the line:
        myTap.Init(pin_clk, pin_io); 
where pin_clk is the pin reference for the interrupt pin you selected and pin_io is the pin reference for the data. That's it!

### Compile your new Arduino project
If the library is properly included into your project, the arduino file should be made without problem after adding call: myTap.Init().
If the library does not compile, because the S3P_conf.h generated by IoTize Studio is not found. The causes could be:
 - the path is not correct because you didn't structure the folders as expected. Modify in tap.cpp the line: 
            #include "..\..\..\TapNLink\S3P_conf.h"
to make S3P_conf.h accessible. 
 - S3P_conf.h has not been generated because S3P protocol is not selected => select the right protocol in 

### Synchronizing "Arduino project <=> configuration"

When saving the configuration file, IoTize Studio generates a S3P_conf.h file that is required for compiling Tap.cpp (from the library). Indeed, this file is needed to generate the final ELF file. But to create the iotz configuration, and to generate this file, we need to have the ELF file...
To break this loop:
 - you can start with your initial ELF file (gnerated before iotizing your project). Once a S3P_conf.h file is generated, il will be used when compiling the Arduino file. 
 - when you assume that your configuration is done, save your project, then compile the project from Arduino file, then repeat this sequence once...

 Note that there is a difference between S3P and SWD:
  - SWD manipulates absolute addresses. Every time you will change (and reprogram) your firmware, you have the risk of a different mapping of the selected variables. It means that if an address has been modified, you must regenerate the configuration in IoTize Studio and then reconfigure your Tap.
  - S3P manipulates indexes that are quite stable. It means that as long as you don't add a new variable accessible by TapNLink, you can keep the same Tap configuration.  

## Configure you Tap

#### Loading the configuration

#### Testing from IoTize Studio

#### Apps publishing

#### Test vith your mobile!

## To go further


