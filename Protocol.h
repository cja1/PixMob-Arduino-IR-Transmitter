/* PixMob Protocol,

   Version 1.0, Septemebr 2024
   Charles Allen

   Based on James Wang's PixMob_IR python code here:
   https://github.com/jamesw343/PixMob_IR

   And on Daniel Weidman's pixmob-ir-reverse-engineering python code here: 
   https://github.com/danielweidman/pixmob-ir-reverse-engineering

*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

class Field {
public:
  uint8_t byte;
  uint8_t offset;
  uint8_t width;
  uint8_t srcOffset;

  Field(uint8_t byte, uint8_t offset, uint8_t width, uint8_t srcOffset = 0);
};

class Command {
public:
  enum Chance {
    CHANCE_100_PCT = 0b000,
    CHANCE_88_PCT  = 0b001,
    CHANCE_67_PCT  = 0b010,
    CHANCE_50_PCT  = 0b011,
    CHANCE_32_PCT  = 0b100,
    CHANCE_16_PCT  = 0b101,
    CHANCE_10_PCT  = 0b110,
    CHANCE_4_PCT   = 0b111
  };

  enum Time {
    TIME_0_MS      = 0b000,
    TIME_32_MS     = 0b001,
    TIME_96_MS     = 0b010,
    TIME_192_MS    = 0b011,
    TIME_480_MS    = 0b100,
    TIME_960_MS    = 0b101,
    TIME_2400_MS   = 0b110,
    TIME_3840_MS   = 0b111
  };

  static const uint8_t kMaxEncodedSize = 73;  // Static max encoded size: 9 x 8 + 1 for Arduino (max size required)
  static const uint8_t kEndFlag = 0;          // End flag for RLE uint_8 array

  Command(uint8_t numBytes, uint8_t flagsType, uint8_t actionId = 0);

  void populateBuffer(Field fields[], uint8_t fieldValues[], uint8_t fieldsLen);
  void encode();  
  void convertToRLE(uint8_t* runLengths);

  static void printUIntArray(String label, uint8_t array[], uint8_t size);

private:
  static const uint8_t kMaxBufferSize = 9;  // Static max buffer size 9 for Arduino (max size required)

  uint8_t buffer[kMaxBufferSize];
  uint8_t numBytes;
  uint8_t flagsType;
  uint8_t actionId;

  uint8_t encodedBits[kMaxEncodedSize];
  uint8_t encodedSize;
};

class CommandSingleColorExt : public Command {
public:
  bool onStart, gstEnable, enableRepeat;
  uint8_t red, green, blue, chance, attack, sustain, release, groupId;

  CommandSingleColorExt(bool onStart = false, bool gstEnable = false, uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t chance = CHANCE_100_PCT,
                        uint8_t attack = TIME_32_MS, uint8_t sustain = TIME_960_MS, uint8_t release = TIME_480_MS, uint8_t groupId = 0, bool enableRepeat = false);

  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fGreen = Field(3, 0, 6, 2);
  Field fRed = Field(4, 0, 6, 2);
  Field fBlue = Field(5, 0, 6, 2);
  Field fChance = Field(6, 0, 3, 0);
  Field fAttack = Field(6, 3, 3, 0);
  Field fSustain = Field(7, 0, 3, 0);
  Field fRelease = Field(7, 3, 3, 0);
  Field fGroupId = Field(8, 0, 5);
  Field fEnableRepeat = Field(8, 5, 1);
};

class CommandSetColor : public Command {
public:
  bool gstEnable, isBackground, skipDisplay;
  uint8_t red, green, blue, profileId, groupId;

  CommandSetColor(bool gstEnable = false, uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t profileId = 0,
                  bool isBackground = false, bool skipDisplay = false, uint8_t groupId = 0);

  void createRLE(uint8_t* runLengths);
  void createStartColorRLE(uint8_t red, uint8_t green, uint8_t blue, uint8_t profileId, uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fGreen = Field(3, 0, 6, 2);
  Field fRed = Field(4, 0, 6, 2);
  Field fBlue = Field(5, 0, 6, 2);
  Field fProfileId = Field(6, 0, 4);
  Field fIsBackground = Field(6, 4, 1);
  Field fSkipDisplay = Field(6, 5, 1);
  Field fGroupId = Field(8, 0, 5);
};

class CommandSetConfig : public Command {
public:
  bool onStart, gstEnable, isRandom;
  uint8_t profileIdLo, profileIdHi, attack, sustain, release;

  CommandSetConfig(bool onStart = true, bool gstEnable = false, uint8_t profileIdLo = 0, uint8_t profileId_Hi = 0, bool isRandom = false, uint8_t attack = 0,
                   uint8_t sustain = 0, uint8_t release = 0);

  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fProfileIdLo = Field(3, 0, 4);
  Field fProfileIdHi1 = Field(3, 4, 2);   //For some reason we have to set fProfileIdHi twice - in two separate fields
  Field fProfileIdHi2 = Field(4, 0, 2, 2);
  Field fIsRandom = Field(4, 2, 1);
  Field fAttack = Field(4, 3, 3, 0);
  Field fSustain = Field(5, 0, 3, 0);
  Field fRelease = Field(5, 3, 3, 0);
};

class CommandSingleColor : public Command {
public:
  bool onStart, gstEnable;
  uint8_t red, green, blue;

  CommandSingleColor(bool onStart = false, bool gstEnable = false, uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0);
  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fGreen = Field(3, 0, 6, 2);
  Field fRed = Field(4, 0, 6, 2);
  Field fBlue = Field(5, 0, 6, 2);
};

class CommandTwoColor : public Command {
public:
  bool gstEnable;
  uint8_t red1, green1, blue1, red2, green2, blue2;

  CommandTwoColor(bool gstEnable = false, uint8_t red1 = 0, uint8_t green1 = 0, uint8_t blue1 = 0, 
                     uint8_t red2 = 0, uint8_t green2 = 0, uint8_t blue2 = 0);
  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fGreen1 = Field(3, 0, 6, 2);
  Field fRed1 = Field(4, 0, 6, 2);
  Field fBlue1 = Field(5, 0, 6, 2);
  Field fGreen2 = Field(6, 0, 6, 2);
  Field fRed2 = Field(7, 0, 6, 2);
  Field fBlue2 = Field(8, 0, 6, 2);
};

class CommandSetRepeatDelayTime : public Command {
public:
  bool gstEnable;
  uint8_t repeatDelay, groupId;

  CommandSetRepeatDelayTime(bool gstEnable = false, uint8_t repeatDelay = 0, uint8_t groupId = 0);
  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fRepeatDelay = Field(6, 0, 3);
  Field fGroupId = Field(8, 0, 3);
};

class CommandSetRepeatCount : public Command {
public:
  bool gstEnable;
  uint8_t repeatCount, groupId;

  CommandSetRepeatCount(bool gstEnable = false, uint8_t repeatCount = 0, uint8_t groupId = 0);
  void createRLE(uint8_t* runLengths);

private:
  Field fOnStart = Field(2, 0, 1);
  Field fGSTEnable = Field(2, 4, 1);
  Field fRepeatCount1 = Field(5, 0, 6);
  Field fRepeatCount2 = Field(6, 0, 2, 6);
  Field fGroupId = Field(8, 0, 5);
};

#endif
