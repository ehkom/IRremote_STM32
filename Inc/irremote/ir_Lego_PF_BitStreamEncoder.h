
//==============================================================================
//    L       EEEEEE   EEEE    OOOO
//    L       E       E       O    O
//    L       EEEE    E  EEE  O    O
//    L       E       E    E  O    O    LEGO Power Functions
//    LLLLLL  EEEEEE   EEEE    OOOO     Copyright (c) 2016, 2017 Philipp Henkel
//==============================================================================

//+=============================================================================
//
//static const uint16_t LOW_BIT_DURATION = 421;
//static const uint16_t HIGH_BIT_DURATION = 711;
//static const uint16_t START_BIT_DURATION = 1184;
//static const uint16_t STOP_BIT_DURATION = 1184;
//static const uint8_t IR_MARK_DURATION = 158;
//static const uint16_t HIGH_PAUSE_DURATION = HIGH_BIT_DURATION - IR_MARK_DURATION;
//static const uint16_t LOW_PAUSE_DURATION = LOW_BIT_DURATION - IR_MARK_DURATION;
//static const uint16_t START_PAUSE_DURATION = START_BIT_DURATION - IR_MARK_DURATION;
//static const uint16_t STOP_PAUSE_DURATION = STOP_BIT_DURATION - IR_MARK_DURATION;
//static const uint8_t MESSAGE_BITS = 18;
//static const uint16_t MAX_MESSAGE_LENGTH = 16000;
#define LOW_BIT_DURATION     421
#define HIGH_BIT_DURATION    711
#define START_BIT_DURATION   1184
#define STOP_BIT_DURATION    1184
#define IR_MARK_DURATION     158
#define HIGH_PAUSE_DURATION  (HIGH_BIT_DURATION - IR_MARK_DURATION)
#define LOW_PAUSE_DURATION   (LOW_BIT_DURATION - IR_MARK_DURATION)
#define START_PAUSE_DURATION (START_BIT_DURATION - IR_MARK_DURATION)
#define STOP_PAUSE_DURATION  (STOP_BIT_DURATION - IR_MARK_DURATION)
#define MESSAGE_BITS         18
#define MAX_MESSAGE_LENGTH   16000


typedef struct
{
    uint16_t data;
    unsigned char repeatMessage;
    uint8_t messageBitIdx;
    uint8_t repeatCount;
    uint16_t messageLength;
}LegoPfBitStreamEncoder_t;

LegoPfBitStreamEncoder_t LegoPfBitStreamEncoder;

// HIGH data bit = IR mark + high pause
// LOW data bit = IR mark + low pause


void LegoPf_sendLegoPowerFunctions(uint16_t data, unsigned char repeat);
void LegoPf_reset(uint16_t data, unsigned char repeatMessage);
int LegoPf_getChannelId();
uint16_t LegoPf_getMessageLength();
unsigned char LegoPf_next();
uint8_t LegoPf_getMarkDuration();
uint32_t LegoPf_getPauseDuration();
