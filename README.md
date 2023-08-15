# Dagu ComMotion Firmware Source Code
## Introduction
The Dagu ComMotion is an Arduino robotics shield designed by Russell Cameron for
Dagu Electronic. The ComMotion combines the functions of an intelligent four
motor controller with a multi-function communications controller - hence
Com-Motion. The ComMotion includes two dedicated on-board micro-controllers that
perform the ComMotion functions.

This repo contains the source to the microcontrollers' firmware. It was posted
here and given a suitable open source license with Russell's kind permission.
Many Thanks Russell.

I am posting the source here for three reasons:

1. To give a versioned history of the firmware as developed by Russell.

2. To host some much-needed bug fixes which I have made to the firmware. Russell
moved on from Dagu before being able to complete those fixes himself.

3. To host a branch of the source including some new features.

## Using this source
Note: as a new requirement, in order to compile recent versions of this code you also need to pull and install the [Dagu_ComMotionCommon](https://github.com/sgparry/Dagu_ComMotionCommon) repo. This contains the common definitions between the firmware and the host, which are also used in an as yet incomplete host side library.

In order to correctly use this source, you will need to consult page 7 of the
user manual and the programming tutorial, both are linked from this page:
http://www.dagurobot.com/goods.php?id=174 At time of writing,

the actual manuals were located here:

https://www.dropbox.com/s/qjet3bj96llqzgg/ComMotion%20Programming%20Tutorial.pdf?dl=1
https://www.dropbox.com/s/udh6bdbq1ff7xma/ComMotion%20Instruction%20Manual.pdf?dl=1

However, a brief informative overview follows.

### Overview of the programming
process To understand the programming process, you have to understand what the
ComMotion is and how it works.

The ComMotion is an Arduino shield, designed to sit on top of an Arduino ATMEGA
microcontroller. However, the ComMotion includes two of these ATMEGA
microcontrollers of its own. You could say that you have three Arduinos doing
your work for you instead of one.

In normal operation, the communication between the three controllers takes place
using i2c protocol. The main host (the Arduino) talks to the first of the
ComMotion controllers - MCU1. MCU1 then relays the same commands via I2C to
MCU2. The two controllers then work together, operating one side of the vehicle
each. How they do all this and what they do in response to each command is
controlled by the code in this firmware.

When you want to update or reprogram the firmware however, the three MCUs have
to communicate differently:

1. You use the Arduino IDE to compile and upload a special sketch to your main
Arduino. This sketch allows the Arduino to act as a programmer, ie. to burn new
firmware to another MCU. This sketch has to be designed to work on your main
Arduino and compiled for it, i.e. you will need to set the board and CPU to
match your _main_ Arduino. There are a number of possible scripts for this job -
including one that is provided as an example Arduino script and one provided by
Dagu. You may need to adapt to suit your specific model of Arduino.

2. You use the Arduino IDE to compile the new firmware with board and CPU set
just as you would if you were compiling it for _Arduino Uno_.

3. You set a jumper and switch on the ComMotion that opens a 'back channel' from
pin 10 on the main Arduino to the programming pin on one of the ComMotion MCUs.

4. You upload from the IDE using ArduinoISP as the programmer setting. This will
then relay the firmware via the main Arduino.

5. Change the switch position and repeat 3 - 5

This fiddly process can be streamlined in a number of ways:
- you can find instructions online for 'localising' the board / CPU settings to
a particular project to avoid having to chop and change between settings when
compiling - this author has successfully used that approach.

- If you have sufficient flash space, you can include the programmer code as a
jumper or switch selectable routine within your main project, allowing you to
switch to programmer mode without further uploading - this author uses that
approach.

- In theory, if you are even more flush with flash space, you could include the
ComMotion firmware as a blob within your main firmware. To get the firmware on
the ComMotion MCUs you could then check versions / checksums and if needed
automatically switch to programming mode, also resetting the ComMotion. You then
need only flip the switch and reset a second time to get the firmware on the
second side of the ComMotion. I believe this approach may have been used on some
Dagu products.

- In theory, you could boot the ComMotion from the code stored in the main 
Arduino flash via i2c using a custom bootloader or stub.
