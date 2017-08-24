#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                             JJJJJ  V   V   CCCC
//                               J    V   V  C
//                               J     V V   C
//                             J J     V V   C
//                              J       V     CCCC
//==============================================================================

#define JVC_BITS           16
#define JVC_HDR_MARK     8000
#define JVC_HDR_SPACE    4000
#define JVC_BIT_MARK      600
#define JVC_ONE_SPACE    1600
#define JVC_ZERO_SPACE    550
#define JVC_RPT_LENGTH  60000

//+=============================================================================
// JVC does NOT repeat by sending a separate code (like NEC does).
// The JVC protocol repeats by skipping the header.
// To send a JVC repeat signal, send the original code value
//   and set 'repeat' to true
//
#if SEND_JVC
unsigned char  sendJVC (unsigned long data,  int nbits,  unsigned char repeat)
{
	// Set IR carrier frequency
    irsend_enableIROut(38);

	// Only send the Header if this is NOT a repeat command
	if (!repeat){
        irsend_mark(JVC_HDR_MARK);
        irsend_space(JVC_HDR_SPACE);
	}

	// Data
    unsigned long  mask;
    for (mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
            irsend_mark(JVC_BIT_MARK);
            irsend_space(JVC_ONE_SPACE);
		} else {
            irsend_mark(JVC_BIT_MARK);
            irsend_space(JVC_ZERO_SPACE);
		}
	}

	// Footer
    irsend_mark(JVC_BIT_MARK);
    irsend_space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
#if DECODE_JVC
unsigned char  decodeJVC (struct decode_results *results)
{
	long  data   = 0;
	int   offset = 1; // Skip first space

	// Check for repeat
	if (  (irparams.rawlen - 1 == 33)
	    && MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK)
	    && MATCH_MARK(results->rawbuf[irparams.rawlen-1], JVC_BIT_MARK)
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = JVC;
        return TRUE;
	}

	// Initial mark
    if (!MATCH_MARK(results->rawbuf[offset++], JVC_HDR_MARK))  return FALSE ;

    if (irparams.rawlen < (2 * JVC_BITS) + 1 )  return FALSE ;

	// Initial space
    if (!MATCH_SPACE(results->rawbuf[offset++], JVC_HDR_SPACE))  return FALSE ;

	for (int i = 0;  i < JVC_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], JVC_BIT_MARK))  return FALSE ;

		if      (MATCH_SPACE(results->rawbuf[offset], JVC_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], JVC_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                            return FALSE ;
		offset++;
	}

	// Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK))  return FALSE ;

	// Success
	results->bits        = JVC_BITS;
	results->value       = data;
	results->decode_type = JVC;

    return TRUE;
}
#endif

