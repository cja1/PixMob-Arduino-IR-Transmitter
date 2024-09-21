#include <Arduino.h>
#include <IRremote.hpp>
#include "Protocol.h"

static const uint8_t kIRTransmitterPin = 4;   // Data pin for the IR transmitter
static const uint8_t kIRFrequencyKHz = 38;    // IR transmitter frequency
static const uint16_t kIRPulseLength = 694;   // IR pulse length

//Global irCommand array of uint_8s
uint8_t irCommand[Command::kMaxEncodedSize];

//Global colors array (max 10)
struct colorObj {
  uint8_t red, green, blue;
};
colorObj colors[10];

void setup() {
  Serial.begin(9600);
  IrSender.begin(kIRTransmitterPin);
  IrSender.enableIROut(kIRFrequencyKHz);
}

void sendIRCommand() {
  uint16_t rawData[Command::kMaxEncodedSize];
  uint8_t i = 0;
  while (irCommand[i] != Command::kEndFlag) {
    rawData[i] = irCommand[i] * kIRPulseLength;
    i++;
  }
  IrSender.sendRaw(rawData, i, kIRFrequencyKHz);
  delay(5);
}

void sendColorPulse(uint8_t red, uint8_t green, uint8_t blue) {
  CommandSingleColorExt commandSingleColorExt = CommandSingleColorExt(false, false, red, green, blue, Command::CHANCE_100_PCT,
                                                                      Command::TIME_32_MS, Command::TIME_96_MS, Command:: TIME_32_MS);
  commandSingleColorExt.createRLE(irCommand);
  sendIRCommand();
}

void sendStartColorSequence(uint8_t numColors, bool isRandom, uint8_t attack, uint8_t sustain, uint8_t release) {
  CommandSetColor commandSetColor = CommandSetColor();

  //Send colors with unique profileIds
  for (uint8_t i = 0; i < numColors; i++) {
    commandSetColor.createStartColorRLE(colors[i].red, colors[i].green, colors[i].blue, i, irCommand);
    sendIRCommand();
  }
  //Set config to trigger the colors in the profiles
  CommandSetConfig commandSetConfig = CommandSetConfig(true, true, 0, numColors - 1, isRandom, attack, sustain, release);
  commandSetConfig.createRLE(irCommand);
  sendIRCommand();
}

void setBackgroundColor(uint8_t red, uint8_t green, uint8_t blue) {
  colors[0].red = red; colors[0].green = green; colors[0].blue = blue;
  sendStartColorSequence(1, false, Command::TIME_960_MS, Command::TIME_960_MS, Command::TIME_0_MS);
}
void clearBackgroundColor() {
  colors[0].red = 0; colors[0].green = 0; colors[0].blue = 0;
  sendStartColorSequence(1, false, Command::TIME_960_MS, Command::TIME_960_MS, Command::TIME_0_MS);
}

void sendRepeatEffect() {
  //Repeat gap 960ms
  CommandSetRepeatDelayTime commandSetRepeatDelayTime = CommandSetRepeatDelayTime(false, Command::TIME_960_MS);
  commandSetRepeatDelayTime.createRLE(irCommand);
  sendIRCommand();

  //Repeat 3 times
  CommandSetRepeatCount commandSetRepeatCount = CommandSetRepeatCount(false, 3);
  commandSetRepeatCount.createRLE(irCommand);
  sendIRCommand();

  //Setthe color purple with enableRepeat set to true
  CommandSingleColorExt commandSingleColorExt = CommandSingleColorExt(false, false, 128, 0, 128, Command::CHANCE_100_PCT,
                                                                      Command::TIME_96_MS, Command::TIME_480_MS, Command:: TIME_96_MS, 0, true);
  commandSingleColorExt.createRLE(irCommand);
  sendIRCommand();
}

void sendErasGoHome() {
  colors[0].red = 0xc0; colors[0].green = 0x64; colors[0].blue = 0x64;
  colors[1].red = 0x98; colors[1].green = 0xc0; colors[1].blue = 0x30;
  colors[2].red = 0x64; colors[2].green = 0xc0; colors[2].blue = 0xc0;
  colors[3].red = 0x7c; colors[3].green = 0xc0; colors[3].blue = 0xc0;
  colors[4].red = 0x1b; colors[4].green = 0xc0; colors[4].blue = 0x1b;
  colors[5].red = 0xc0; colors[5].green = 0x64; colors[5].blue = 0xc0;
  colors[6].red = 0x00; colors[6].green = 0x2b; colors[6].blue = 0xc0;
  colors[7].red = 0x38; colors[7].green = 0x38; colors[7].blue = 0x38;
  sendStartColorSequence(8, true, Command::TIME_480_MS, Command::TIME_480_MS, Command::TIME_480_MS);
}

//Demo
void loop() {
  //4 pulses: white, red, green, blue
  delay(1000);
  sendColorPulse(255,255,255);
  delay(2000);
  sendColorPulse(255,0,0);
  delay(2000);
  sendColorPulse(0,255,0);
  delay(2000);
  sendColorPulse(0,0,255);
  delay(5000);

  //Send repeated purple color x3 using repeat effect
  sendRepeatEffect();
  delay(5000);

  //Set background color then clear
  setBackgroundColor(0, 255, 128);
  delay(5000);
  clearBackgroundColor();
  delay(1000);

  //Set 8 colors that randomly cycle
  sendErasGoHome();
  
  while(1);
}
