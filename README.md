# PixMob IR Arduino

This project provides standalone Arduino code to control a PixMob infrared (IR) wristband.

Previous PixMob projects have used an Arduino to listen on the serial port for IR commands then transmit the IR commands to a PixMob using an IR LED. With this approach 2 devices are needed: an Arduino and a separate computer to send commands over the serial port.

This project implements all the logic for creating and sending PixMob commands in Arduino C++ code.

The code is primarily based on [James Wang](https://github.com/jamesw343)'s work that reverse engineered the operation modes and communication protocol for IR PixMob wristbands. James Wang's [code](https://github.com/jamesw343/PixMob_IR) is written in Python and runs on a separate computer from the Arduino. In this repo the Python code has been converted to Arduino C++ so an Arduino can create and send IR commands without requiring a separate computer.

## Use
1. Download the 3 files in this repo: trasmitter.ino, Protocol.h and Protocol.cpp. Put them in your Arduino directory.
2. Follow the instructions on Daniel Weidman's [pixmob-ir-reverse-engineering](https://github.com/danielweidman/pixmob-ir-reverse-engineering/) page to connect an IR LED to the Arduino.
3. Upload and run the transmitter.ino code on the Arduino. You will see a series of lighting effects appear on nearby PixMob wristbands.

## Limitations
Not all of the commands in James Wang's [code](https://github.com/jamesw343/PixMob_IR) have been migrated across to Arduino. Key commands to make the PixMob bracelet show different lighting effects are included. Details:

|Included|					Excluded|
| ----	|					----	|
|CommandSingleColorExt|		CommandSetGroupSel			|
|CommandSetColor|			CommandSetGroupId			|
|CommandSetConfig|			CommandSetGlobalSustainTime |
|CommandSingleColor|		CommandIdentFWVersion 		|
|CommandTwoColors| 			CommandDoReset 				|
|CommandSetRepeatDelayTime|								|
|CommandSetRepeatCount|									|


## Acknowledgements

* [@jamesw343](https://github.com/jamesw343), [jamesw343/PixMob_IR](https://github.com/jamesw343/PixMob_IR) for his Python implementation of the messaging protocol to send many different commands to a PixMob IR wristband.
* [@danielweidman](https://github.com/danielweidman), [danielweidman/pixmob-ir-reverse-engineering](https://github.com/danielweidman/pixmob-ir-reverse-engineering/), and contributors for effect definitions and Arduino IR sender program

For more PixMob-related projects, take a look at the PixMob discord server: [https://discord.gg/UYqTjC7xp3](https://discord.gg/UYqTjC7xp3)

