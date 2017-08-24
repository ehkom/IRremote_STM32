#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                           N   N  EEEEE   CCCC
//                           NN  N  E      C
//                           N N N  EEE    C
//                           N  NN  E      C
//                           N   N  EEEEE   CCCC
//==============================================================================

#define NEC_BITS          32
#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK     560
#define NEC_ONE_SPACE   1690
#define NEC_ZERO_SPACE   560
#define NEC_RPT_SPACE   2250

//+=============================================================================
#if SEND_NEC
void  sendNEC (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
    irsend_enableIROut(38);

	// Header
    irsend_mark(NEC_HDR_MARK);
    irsend_space(NEC_HDR_SPACE);

	// Data
    unsigned long  mask;
    for (mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
            irsend_mark(NEC_BIT_MARK);
            irsend_space(NEC_ONE_SPACE);
		} else {
            irsend_mark(NEC_BIT_MARK);
            irsend_space(NEC_ZERO_SPACE);
		}
	}

	// Footer
    irsend_mark(NEC_BIT_MARK);
    irsend_space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// NECs have a repeat only 4 items long
//
#if DECODE_NEC
unsigned char  decodeNEC (struct decode_results *results)
{
	long  data   = 0;  // We decode in to here; Start with nothing
	int   offset = 1;  // Index in to results; Skip first entry!?

	// Check header "mark"
    if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK))  return FALSE ;
	offset++;

	// Check for repeat
	if ( (irparams.rawlen == 4)
	    && MATCH_SPACE(results->rawbuf[offset  ], NEC_RPT_SPACE)
	    && MATCH_MARK (results->rawbuf[offset+1], NEC_BIT_MARK )
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = NEC;
        return TRUE;
	}

	// Check we have enough data
    if (irparams.rawlen < (2 * NEC_BITS) + 4)  return FALSE ;

	// Check header "space"
    if (!MATCH_SPACE(results->rawbuf[offset], NEC_HDR_SPACE))  return FALSE ;
	offset++;

	// Build the data
	for (int i = 0;  i < NEC_BITS;  i++) {
		// Check data "mark"
        if (!MATCH_MARK(results->rawbuf[offset], NEC_BIT_MARK))  return FALSE ;
		offset++;
        // Suppend this bit
		if      (MATCH_SPACE(results->rawbuf[offset], NEC_ONE_SPACE ))  data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], NEC_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                            return FALSE ;
		offset++;
	}

	// Success
	results->bits        = NEC_BITS;
	results->value       = data;
	results->decode_type = NEC;

    return TRUE;
}
#endif
