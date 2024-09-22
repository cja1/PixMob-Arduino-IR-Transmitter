/* PixMob Protocol,

   Charles Allen, September 2024

   Based on James Wang's PixMob_IR python code here:
   https://github.com/jamesw343/PixMob_IR

   And on Daniel Weidman's pixmob-ir-reverse-engineering python code here: 
   https://github.com/danielweidman/pixmob-ir-reverse-engineering

*/

#include "Protocol.h"

Field::Field(uint8_t byte, uint8_t offset, uint8_t width, uint8_t srcOffset)
  : byte(byte), offset(offset), width(width), srcOffset(srcOffset) {}

Command::Command(uint8_t numBytes, uint8_t flagsType, uint8_t actionId)
  : numBytes(numBytes), flagsType(flagsType), actionId(actionId) {}

void Command::populateBuffer(Field fields[], uint8_t fieldValues[], uint8_t fieldsLen) {
  memset(buffer, 0, sizeof(buffer));
  buffer[0] = 0b10000000;         // Magic value
  buffer[2] = flagsType << 1;    // Command type

  if (numBytes == 9 && actionId != 0) {   //If actionId set then use for buffer[7]
    buffer[7] = actionId;
  }

  for (uint8_t i = 0; i < fieldsLen; i++) {
    Field field = fields[i];
    uint8_t fieldValue = fieldValues[i];

    uint8_t fragmentValue = fieldValue >> field.srcOffset;
    fragmentValue &= (1 << field.width) - 1;
    fragmentValue <<= field.offset;
    buffer[field.byte] |= fragmentValue;
  }
}

void Command::encode() {
  const uint8_t encodingMap[64] PROGMEM = {
    0x21, 0x32, 0x54, 0x65, 0xa9, 0x9a, 0x6d, 0x29,
    0x56, 0x92, 0xa1, 0xb4, 0xb2, 0x84, 0x66, 0x2a,
    0x4c, 0x6a, 0xa6, 0x95, 0x62, 0x51, 0x42, 0x24,
    0x35, 0x46, 0x8a, 0xac, 0x8c, 0x6c, 0x2c, 0x4a,
    0x59, 0x86, 0xa4, 0xa2, 0x91, 0x64, 0x55, 0x44,
    0x22, 0x31, 0xb1, 0x52, 0x85, 0x96, 0xa5, 0x69,
    0x5a, 0x2d, 0x4d, 0x89, 0x45, 0x34, 0x61, 0x25,
    0x36, 0xad, 0x94, 0xaa, 0x8d, 0x49, 0x99, 0x26
  };

  uint8_t encodedBytes[kMaxBufferSize];
  memcpy(encodedBytes, buffer, kMaxBufferSize);
  
  uint16_t checksum = 0;
  for (uint8_t i = 2; i < numBytes; i++) {
    encodedBytes[i] = encodingMap[encodedBytes[i]];
    checksum += encodedBytes[i];
  }

  checksum = (checksum >> 2) & 0x3F;
  encodedBytes[1] = encodingMap[checksum];

  uint8_t bitIndex = 0;
  bool leadingOneFound = false;
  for (uint8_t i = 0; i < sizeof(encodedBytes); i++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t bit = (encodedBytes[i] >> j) & 0x01;
      if (bit == 1) {
        leadingOneFound = true;
      }
      if (leadingOneFound) {
        encodedBits[bitIndex++] = bit;
      }
    }
  }

  while (encodedBits[bitIndex - 1] == 0) {
    bitIndex--;
  }

  encodedSize = bitIndex;
}

//Run Length Encoded unsigned short uint8_t array, so 1,1,0,0,0,1,1,1,0 => {2,3,3,1}
void Command::convertToRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  uint8_t runCount = 0;
  uint8_t currentBit = encodedBits[0];
  uint8_t runLength = 1;

  for (uint8_t i = 1; i < encodedSize; i++) {
    if (encodedBits[i] == currentBit) {
      runLength++;
    } else {
      RLEArray[runCount++] = runLength;
      currentBit = encodedBits[i];
      runLength = 1;
    }
  }
  RLEArray[runCount++] = runLength;
  *RLEArrayLen = runCount;
}

//Debug tools
void Command::printUInt8Array(String label, uint8_t array[], uint8_t size) {
  Serial.print(label + ", size " + size + ": ");
  for (uint8_t i = 0; i < size; i++) {
    if (i > 0) {
      Serial.print(",");
    }
    Serial.print(array[i], HEX);
  }
  Serial.println("");
}
void Command::printUInt16Array(String label, uint16_t array[], uint8_t size) {
  Serial.print(label + ", size " + size + ": ");
  for (uint8_t i = 0; i < size; i++) {
    if (i > 0) {
      Serial.print(",");
    }
    Serial.print(array[i], HEX);
  }
  Serial.println("");
}

CommandSingleColorExt::CommandSingleColorExt(bool onStart, bool gstEnable, uint8_t red, uint8_t green, uint8_t blue, uint8_t chance = CHANCE_100_PCT,
                                             uint8_t attack = TIME_32_MS, uint8_t sustain = TIME_960_MS, uint8_t release = TIME_480_MS,
                                             uint8_t groupId = 0, bool enableRepeat = 0)
  : onStart(onStart), gstEnable(gstEnable), red(red), green(green), blue(blue), chance(chance), attack(attack),
    sustain(sustain), release(release), groupId(groupId), enableRepeat(enableRepeat), Command(9, 0b000) {}

void CommandSingleColorExt::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[11] = { fOnStart, fGSTEnable, fGreen, fRed, fBlue, fChance, fAttack, fSustain, fRelease, fGroupId, fEnableRepeat };
  
  uint8_t values[11] = { onStart, gstEnable, green, red, blue, chance, attack, sustain, release, groupId, enableRepeat };
  Command::populateBuffer(fields, values, 11);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}

CommandSetColor::CommandSetColor(bool gstEnable, uint8_t red, uint8_t green, uint8_t blue, uint8_t profileId,
                                 bool isBackground, bool skipDisplay, uint8_t groupId)
  : gstEnable(gstEnable), red(red), green(green), blue(blue), profileId(profileId), isBackground(isBackground),
    skipDisplay(skipDisplay), groupId(groupId), Command(9, 0b111) {}

void CommandSetColor::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[9] = { fOnStart, fGSTEnable, fGreen, fRed, fBlue, fProfileId, fIsBackground, fSkipDisplay, fGroupId };

  //Fix onStart to true (read_only, and only true allowed)
  uint8_t values[9] = { true, gstEnable, green, red, blue, profileId, isBackground, skipDisplay, groupId };
  populateBuffer(fields, values, 9);
  encode();
  convertToRLE(RLEArray, RLEArrayLen);
}

void CommandSetColor::createStartColorRLE(uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _profileId, uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  red = _red;
  green = _green;
  blue = _blue;
  profileId = _profileId;
  isBackground = 0;
  skipDisplay = 1;
  createRLE(RLEArray, RLEArrayLen);
}

CommandSetConfig::CommandSetConfig(bool onStart, bool gstEnable, uint8_t profileIdLo, uint8_t profileIdHi, bool isRandom, uint8_t attack,
                                   uint8_t sustain, uint8_t release)
  : onStart(onStart), gstEnable(gstEnable), profileIdLo(profileIdLo), profileIdHi(profileIdHi), isRandom(isRandom),
    attack(attack), sustain(sustain), release(release), Command(6, 0b001) {}

void CommandSetConfig::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  //Send profileIdHi1 and profileIdHi2
  Field fields[9] = { fOnStart, fGSTEnable, fProfileIdLo, fProfileIdHi1, fProfileIdHi2, fIsRandom, fAttack, fSustain, fRelease };

  //Send profileIdHi twice: maps to fProfileIdHi1 and fProfileIdHi2
  uint8_t values[9] = { onStart, gstEnable, profileIdLo, profileIdHi, profileIdHi, isRandom, attack, sustain, release };
  Command::populateBuffer(fields, values, 9);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}

CommandSingleColor::CommandSingleColor(bool onStart, bool gstEnable, uint8_t red, uint8_t green, uint8_t blue)
  : onStart(onStart), gstEnable(gstEnable), red(red), green(green), blue(blue), Command(6, 0b000) {}

void CommandSingleColor::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[5] = { fOnStart, fGSTEnable, fGreen, fRed, fBlue };
  
  uint8_t values[5] = { onStart, gstEnable, green, red, blue };
  Command::populateBuffer(fields, values, 5);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}

CommandTwoColor::CommandTwoColor(bool gstEnable, uint8_t red1, uint8_t green1, uint8_t blue1,
                                 uint8_t red2, uint8_t green2, uint8_t blue2)
  : gstEnable(gstEnable), red1(red1), green1(green1), blue1(blue1),
    red2(red2), green2(green2), blue2(blue2), Command(9, 0b010) {}

void CommandTwoColor::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[8] = { fOnStart, fGSTEnable, fGreen1, fRed1, fBlue1, fGreen2, fRed2, fBlue2 };
  
  //onStart always true
  uint8_t values[8] = { true, gstEnable, green1, red1, blue1, green2, red2, blue2 };
  Command::populateBuffer(fields, values, 8);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}

CommandSetRepeatDelayTime::CommandSetRepeatDelayTime(bool gstEnable = false, uint8_t repeatDelay = 0, uint8_t groupId = 0)
  : gstEnable(gstEnable), repeatDelay(repeatDelay), groupId(groupId), Command(9, 0b111, 7) {}

void CommandSetRepeatDelayTime::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[4] = { fOnStart, fGSTEnable, fRepeatDelay, fGroupId };
  
  //onStart always true
  uint8_t values[4] = { true, gstEnable, repeatDelay, groupId };
  Command::populateBuffer(fields, values, 4);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}

CommandSetRepeatCount::CommandSetRepeatCount(bool gstEnable = false, uint8_t repeatCount = 0, uint8_t groupId = 0)
  : gstEnable(gstEnable), repeatCount(repeatCount), groupId(groupId), Command(9, 0b111, 8) {}

void CommandSetRepeatCount::createRLE(uint8_t* RLEArray, uint8_t* RLEArrayLen) {
  Field fields[5] = { fOnStart, fGSTEnable, fRepeatCount1, fRepeatCount2, fGroupId };

  //onStart always true
  uint8_t values[5] = { true, gstEnable, repeatCount, repeatCount, groupId };
  Command::populateBuffer(fields, values, 5);
  Command::encode();
  Command::convertToRLE(RLEArray, RLEArrayLen);
}
