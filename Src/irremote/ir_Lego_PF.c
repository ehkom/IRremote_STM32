#include "IRremote.h"
#include "IRremoteInt.h"
#include "ir_Lego_PF_BitStreamEncoder.h"

//==============================================================================
//    L       EEEEEE   EEEE    OOOO
//    L       E       E       O    O
//    L       EEEE    E  EEE  O    O
//    L       E       E    E  O    O    LEGO Power Functions
//    LLLLLL  EEEEEE   EEEE    OOOO     Copyright (c) 2016 Philipp Henkel
//==============================================================================

//Private function declarations
uint16_t LegoPf_getDataBitPause();
uint32_t LegoPf_getStopPause();
uint32_t LegoPf_getRepeatStopPause();

// Supported Devices
// LEGO® Power Functions IR Receiver 8884

//+=============================================================================
//
#if SEND_LEGO_PF

#if DEBUG
void logFunctionParameters(uint16_t data, bool repeat)
{
  DBG_PRINT("sendLegoPowerFunctions(data=");
  DBG_PRINT(data);
  DBG_PRINT(", repeat=");
  DBG_PRINTLN(repeat?"true)" : "false)");
}
#endif // DEBUG

void LegoPf_sendLegoPowerFunctions(uint16_t data, unsigned char repeat)
{
#if DEBUG
  logFunctionParameters(data, repeat);
#endif // DEBUG

  irsend_enableIROut(38);
  static LegoPfBitStreamEncoder_t bitStreamEncoder;
  LegoPf_reset(LegoPfBitStreamEncoder.data, LegoPfBitStreamEncoder.repeat);
  do {
    mark(bitStreamEncoder.getMarkDuration());
    space(bitStreamEncoder.getPauseDuration());
  } while (bitStreamEncoder.next());
}

void LegoPf_reset(uint16_t data, unsigned char repeatMessage)
{
  this->data = data;
  this->repeatMessage = repeatMessage;
  messageBitIdx = 0;
  repeatCount = 0;
  messageLength = getMessageLength();
}

int LegoPf_getChannelId()
{
    return 1 + ((LegoPfBitStreamEncoder.data >> 12) & 0x3);
}

uint16_t LegoPf_getMessageLength()
{
  // Sum up all marks
  uint16_t length = MESSAGE_BITS * IR_MARK_DURATION;

  // Sum up all pauses
  length += START_PAUSE_DURATION;
  for (unsigned long mask = 1UL << 15; mask; mask >>= 1) {
    if (LegoPfBitStreamEncoder.data & mask) {
      length += HIGH_PAUSE_DURATION;
    } else {
      length += LOW_PAUSE_DURATION;
    }
  }
  length += STOP_PAUSE_DURATION;
  return length;
}

unsigned char LegoPf_next()
{
  messageBitIdx++;
  if (messageBitIdx >= MESSAGE_BITS) {
    repeatCount++;
    messageBitIdx = 0;
  }
  if (repeatCount >= 1 && !repeatMessage) {
    return false;
  } else if (repeatCount >= 5) {
    return false;
  } else {
    return true;
  }
}

uint8_t LegoPf_getMarkDuration()
{
    return IR_MARK_DURATION;
}

uint32_t LegoPf_getPauseDuration()
{
  if (messageBitIdx == 0)
    return START_PAUSE_DURATION;
  else if (messageBitIdx < MESSAGE_BITS - 1) {
    return getDataBitPause();
  } else {
    return getStopPause();
  }
}

uint16_t LegoPf_getDataBitPause()
{
  const int pos = MESSAGE_BITS - 2 - messageBitIdx;
  const bool isHigh = data & (1 << pos);
  return isHigh ? HIGH_PAUSE_DURATION : LOW_PAUSE_DURATION;
}

uint32_t LegoPf_getStopPause()
{
  if (repeatMessage) {
    return getRepeatStopPause();
  } else {
    return STOP_PAUSE_DURATION;
  }
}

uint32_t LegoPf_getRepeatStopPause()
{
  if (repeatCount == 0 || repeatCount == 1) {
    return STOP_PAUSE_DURATION + (uint32_t)5 * MAX_MESSAGE_LENGTH - messageLength;
  } else if (repeatCount == 2 || repeatCount == 3) {
    return STOP_PAUSE_DURATION
           + (uint32_t)(6 + 2 * getChannelId()) * MAX_MESSAGE_LENGTH - messageLength;
  } else {
    return STOP_PAUSE_DURATION;
  }
}

#endif // SEND_LEGO_PF
