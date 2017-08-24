#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//              SSSS   AAA    MMM    SSSS  U   U  N   N   GGGG
//             S      A   A  M M M  S      U   U  NN  N  G
//              SSS   AAAAA  M M M   SSS   U   U  N N N  G  GG
//                 S  A   A  M   M      S  U   U  N  NN  G   G
//             SSSS   A   A  M   M  SSSS    UUU   N   N   GGG
//==============================================================================

#define SAMSUNG_BITS          32
#define SAMSUNG_HDR_MARK    5000
#define SAMSUNG_HDR_SPACE   5000
#define SAMSUNG_BIT_MARK     560
#define SAMSUNG_ONE_SPACE   1600
#define SAMSUNG_ZERO_SPACE   560
#define SAMSUNG_RPT_SPACE   2250

//+=============================================================================
#if SEND_SAMSUNG
void  sendSAMSUNG (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
    irsend_enableIROut(38);

	// Header
    irsend_mark(SAMSUNG_HDR_MARK);
    irsend_space(SAMSUNG_HDR_SPACE);

	// Data
    unsigned long  mask;
    for (mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
            irsend_mark(SAMSUNG_BIT_MARK);
            irsend_space(SAMSUNG_ONE_SPACE);
		} else {
            irsend_mark(SAMSUNG_BIT_MARK);
            irsend_space(SAMSUNG_ZERO_SPACE);
		}
	}

	// Footer
    irsend_mark(SAMSUNG_BIT_MARK);
    irsend_space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
// SAMSUNGs have a repeat only 4 items long
//
#if DECODE_SAMSUNG
unsigned char  decodeSAMSUNG (struct decode_results *results)
{
	long  data   = 0;
	int   offset = 1;  // Skip first space

	// Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], SAMSUNG_HDR_MARK))   return FALSE ;
	offset++;

	// Check for repeat
	if (    (irparams.rawlen == 4)
	     && MATCH_SPACE(results->rawbuf[offset], SAMSUNG_RPT_SPACE)
	     && MATCH_MARK(results->rawbuf[offset+1], SAMSUNG_BIT_MARK)
	   ) {
		results->bits        = 0;
		results->value       = REPEAT;
		results->decode_type = SAMSUNG;
        return TRUE;
	}
    if (irparams.rawlen < (2 * SAMSUNG_BITS) + 4)  return FALSE ;

	// Initial space
    if (!MATCH_SPACE(results->rawbuf[offset++], SAMSUNG_HDR_SPACE))  return FALSE ;

	for (int i = 0;  i < SAMSUNG_BITS;   i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], SAMSUNG_BIT_MARK))  return FALSE ;

		if      (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], SAMSUNG_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                                return FALSE ;
		offset++;
	}

	// Success
	results->bits        = SAMSUNG_BITS;
	results->value       = data;
	results->decode_type = SAMSUNG;
    return TRUE;
}
#endif

