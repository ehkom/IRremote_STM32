#include "IRremote.h"
#include "IRremoteInt.h"

//
//private function definitions
//
long  decodeHash (struct decode_results *results) ;
int   compare    (unsigned int oldval, unsigned int newval) ;
//......................................................................
#		if (DECODE_RC5 || DECODE_RC6)
// This helper function is shared by RC5 and RC6
int  getRClevel (struct decode_results *results,  int *offset,  int *used,  int t1) ;
#		endif
#		if DECODE_RC5
unsigned char  decodeRC5        (struct decode_results *results) ;
#		endif
#		if DECODE_RC6
unsigned char  decodeRC6        (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_NEC
unsigned char  decodeNEC        (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_SONY
unsigned char  decodeSony       (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_PANASONIC
unsigned char  decodePanasonic  (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_JVC
unsigned char  decodeJVC        (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_SAMSUNG
unsigned char  decodeSAMSUNG    (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_WHYNTER
unsigned char  decodeWhynter    (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_AIWA_RC_T501
unsigned char  decodeAiwaRCT501 (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_LG
unsigned char  decodeLG         (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_SANYO
unsigned char  decodeSanyo      (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_MITSUBISHI
unsigned char  decodeMitsubishi (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_DISH
unsigned char  decodeDish (struct decode_results *results) ; // NOT WRITTEN
#		endif
//......................................................................
#		if DECODE_SHARP
unsigned char  decodeSharp (struct decode_results *results) ; // NOT WRITTEN
#		endif
//......................................................................
#		if DECODE_DENON
unsigned char  decodeDenon (struct decode_results *results) ;
#		endif
//......................................................................
#		if DECODE_LEGO_PF
unsigned char  decodeLegoPowerFunctions (struct decode_results *results) ;
#		endif

//+=============================================================================
// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
//
int  irrecv_decode (struct decode_results *results)
{
	results->rawbuf   = irparams.rawbuf;
	results->rawlen   = irparams.rawlen;

	results->overflow = irparams.overflow;

    if (irparams.rcvstate != STATE_STOP)  return FALSE ;

#if DECODE_NEC
	DBG_PRINTLN("Attempting NEC decode");
    if (decodeNEC(results))  return TRUE ;
#endif

#if DECODE_SONY
	DBG_PRINTLN("Attempting Sony decode");
    if (decodeSony(results))  return TRUE ;
#endif

#if DECODE_SANYO
	DBG_PRINTLN("Attempting Sanyo decode");
    if (decodeSanyo(results))  return TRUE ;
#endif

#if DECODE_MITSUBISHI
	DBG_PRINTLN("Attempting Mitsubishi decode");
    if (decodeMitsubishi(results))  return TRUE ;
#endif

#if DECODE_RC5
	DBG_PRINTLN("Attempting RC5 decode");
    if (decodeRC5(results))  return TRUE ;
#endif

#if DECODE_RC6
	DBG_PRINTLN("Attempting RC6 decode");
    if (decodeRC6(results))  return TRUE ;
#endif

#if DECODE_PANASONIC
	DBG_PRINTLN("Attempting Panasonic decode");
    if (decodePanasonic(results))  return TRUE ;
#endif

#if DECODE_LG
	DBG_PRINTLN("Attempting LG decode");
    if (decodeLG(results))  return TRUE ;
#endif

#if DECODE_JVC
	DBG_PRINTLN("Attempting JVC decode");
    if (decodeJVC(results))  return TRUE ;
#endif

#if DECODE_SAMSUNG
	DBG_PRINTLN("Attempting SAMSUNG decode");
    if (decodeSAMSUNG(results))  return TRUE ;
#endif

#if DECODE_WHYNTER
	DBG_PRINTLN("Attempting Whynter decode");
    if (decodeWhynter(results))  return TRUE ;
#endif

#if DECODE_AIWA_RC_T501
	DBG_PRINTLN("Attempting Aiwa RC-T501 decode");
    if (decodeAiwaRCT501(results))  return TRUE ;
#endif

#if DECODE_DENON
	DBG_PRINTLN("Attempting Denon decode");
    if (decodeDenon(results))  return TRUE ;
#endif

#if DECODE_LEGO_PF
	DBG_PRINTLN("Attempting Lego Power Functions");
    if (decodeLegoPowerFunctions(results))  return TRUE ;
#endif

	// decodeHash returns a hash on any input.
	// Thus, it needs to be last in the list.
	// If you add any decodes, add them before this.
    if (decodeHash(results))  return TRUE ;

	// Throw away and start over
	resume();
    return FALSE;
}

//+=============================================================================
//IRrecv::IRrecv (int recvpin)
//{
//	irparams.recvpin = recvpin;
//	irparams.blinkflag = 0;
//}

//IRrecv::IRrecv (int recvpin, int blinkpin)
//{
//	irparams.recvpin = recvpin;
//	irparams.blinkpin = blinkpin;
//	pinMode(blinkpin, OUTPUT);
//	irparams.blinkflag = 0;
//}



//+=============================================================================
// initialization
//
void  irrecv_enableIRIn ( )
{
    GPIO_InitTypeDef GPIO_InitStruct;

// Interrupt Service Routine - Fires every 50uS
#ifdef ESP32
	// ESP32 has a proper API to setup timers, no weird chip macros needed
	// simply call the readable API versions :)
	// 3 timers, choose #1, 80 divider nanosecond precision, 1 to count up
	timer = timerBegin(1, 80, 1);
	timerAttachInterrupt(timer, &IRTimer, 1);
    // every 50ns, autoreload = TRUE
    timerAlarmWrite(timer, 50, TRUE);
	timerAlarmEnable(timer);
#else
	cli();
	// Setup pulse clock timer interrupt
	// Prescale /8 (16M/8 = 0.5 microseconds per tick)
	// Therefore, the timer interval can range from 0.5 to 128 microseconds
	// Depending on the reset value (255 to 0)
	TIMER_CONFIG_NORMAL();

	// Timer2 Overflow Interrupt Enable
	TIMER_ENABLE_INTR;

	TIMER_RESET;

	sei();  // enable interrupts
#endif

	// Initialize state machine variables
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;

    // Set pin mode to input
    GPIO_InitStruct.Pin = irparams.GPIO_PinIR;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(irparams.GPIO_PortIR, &GPIO_InitStruct);
}

//+=============================================================================
// Enable/disable blinking of pin 13 on IR processing
//
void  irrecv_blink13 (int blinkflag)
{
    GPIO_InitTypeDef GPIO_InitStruct;

	irparams.blinkflag = blinkflag;
    if (blinkflag)
    {
        //Set pin mode to output
        GPIO_InitStruct.Pin = irparams.GPIO_PinBlink;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(irparams.GPIO_PortBlink, &GPIO_InitStruct);
    }
}

//+=============================================================================
// Return if receiving new IR signals
//
unsigned char  irrecv_isIdle ( )
{
    return (irparams.rcvstate == STATE_IDLE || irparams.rcvstate == STATE_STOP) ? TRUE : FALSE;
}
//+=============================================================================
// Restart the ISR state machine
//
void  irrecv_resume ( )
{
	irparams.rcvstate = STATE_IDLE;
	irparams.rawlen = 0;
}

//+=============================================================================
// hashdecode - decode an arbitrary IR code.
// Instead of decoding using a standard encoding scheme
// (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
//
// The algorithm: look at the sequence of MARK signals, and see if each one
// is shorter (0), the same length (1), or longer (2) than the previous.
// Do the same with the SPACE signals.  Hash the resulting sequence of 0's,
// 1's, and 2's to a 32-bit value.  This will give a unique value for each
// different code (probably), for most code systems.
//
// http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
//
// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
//
int  irrecv_compare (unsigned int oldval,  unsigned int newval)
{
	if      (newval < oldval * .8)  return 0 ;
	else if (oldval < newval * .8)  return 2 ;
	else                            return 1 ;
}

//+=============================================================================
// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
// Converts the raw code values into a 32-bit hash code.
// Hopefully this code is unique for each button.
// This isn't a "real" decoding, just an arbitrary value.
//
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

long  irrecv_decodeHash (struct decode_results *results)
{
	long  hash = FNV_BASIS_32;

	// Require at least 6 samples to prevent triggering on noise
    if (results->rawlen < 6)  return FALSE ;

	for (int i = 1;  (i + 2) < results->rawlen;  i++) {
		int value =  compare(results->rawbuf[i], results->rawbuf[i+2]);
		// Add value into the hash
		hash = (hash * FNV_PRIME_32) ^ value;
	}

	results->value       = hash;
	results->bits        = 32;
	results->decode_type = UNKNOWN;

    return TRUE;
}
