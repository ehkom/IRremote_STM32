#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                           SSSS   OOO   N   N  Y   Y
//                          S      O   O  NN  N   Y Y
//                           SSS   O   O  N N N    Y
//                              S  O   O  N  NN    Y
//                          SSSS    OOO   N   N    Y
//==============================================================================

#define SONY_BITS                   12
#define SONY_HDR_MARK             2400
#define SONY_HDR_SPACE             600
#define SONY_ONE_MARK             1200
#define SONY_ZERO_MARK             600
#define SONY_RPT_LENGTH          45000
#define SONY_DOUBLE_SPACE_USECS    500  // usually ssee 713 - not using ticks as get number wrapround

//+=============================================================================
#if SEND_SONY
void  sendSony (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
    irsend_enableIROut(40);

	// Header
    irsend_mark(SONY_HDR_MARK);
    irsend_space(SONY_HDR_SPACE);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
            irsend_mark(SONY_ONE_MARK);
            irsend_space(SONY_HDR_SPACE);
		} else {
            irsend_mark(SONY_ZERO_MARK);
            irsend_space(SONY_HDR_SPACE);
    	}
  	}

	// We will have ended with LED off
}
#endif

//+=============================================================================
#if DECODE_SONY
unsigned char  decodeSony (struct decode_results *results)
{
	long  data   = 0;
	int   offset = 0;  // Dont skip first space, check its size

    if (irparams.rawlen < (2 * SONY_BITS) + 2)  return FALSE ;

	// Some Sony's deliver repeats fast after first
	// unfortunately can't spot difference from of repeat from two fast clicks
	if (results->rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
		// Serial.print("IR Gap found: ");
		results->bits = 0;
		results->value = REPEAT;

#	ifdef DECODE_SANYO
		results->decode_type = SANYO;
#	else
		results->decode_type = UNKNOWN;
#	endif

        return TRUE;
	}
	offset++;

	// Initial mark
    if (!MATCH_MARK(results->rawbuf[offset++], SONY_HDR_MARK))  return FALSE ;

	while (offset + 1 < irparams.rawlen) {
		if (!MATCH_SPACE(results->rawbuf[offset++], SONY_HDR_SPACE))  break ;

		if      (MATCH_MARK(results->rawbuf[offset], SONY_ONE_MARK))   data = (data << 1) | 1 ;
		else if (MATCH_MARK(results->rawbuf[offset], SONY_ZERO_MARK))  data = (data << 1) | 0 ;
        else                                                           return FALSE ;
		offset++;
	}

	// Success
	results->bits = (offset - 1) / 2;
	if (results->bits < 12) {
		results->bits = 0;
        return FALSE;
	}
	results->value       = data;
	results->decode_type = SONY;
    return TRUE;
}
#endif

