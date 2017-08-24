#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                               L       GGGG
//                               L      G
//                               L      G  GG
//                               L      G   G
//                               LLLLL   GGG
//==============================================================================

#define LG_BITS 28

#define LG_HDR_MARK 8000
#define LG_HDR_SPACE 4000
#define LG_BIT_MARK 600
#define LG_ONE_SPACE 1600
#define LG_ZERO_SPACE 550
#define LG_RPT_LENGTH 60000

//+=============================================================================
#if DECODE_LG
unsigned char  decodeLG (struct decode_results *results)
{
    long  data   = 0;
    int   offset = 1; // Skip first space

	// Check we have the right amount of data
    if (irparams.rawlen < (2 * LG_BITS) + 1 )  return FALSE ;

    // Initial mark/space
    if (!MATCH_MARK(results->rawbuf[offset++], LG_HDR_MARK))  return FALSE ;
    if (!MATCH_SPACE(results->rawbuf[offset++], LG_HDR_SPACE))  return FALSE ;

    for (int i = 0;  i < LG_BITS;  i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], LG_BIT_MARK))  return FALSE ;

        if      (MATCH_SPACE(results->rawbuf[offset], LG_ONE_SPACE))   data = (data << 1) | 1 ;
        else if (MATCH_SPACE(results->rawbuf[offset], LG_ZERO_SPACE))  data = (data << 1) | 0 ;
        else                                                           return FALSE ;
        offset++;
    }

    // Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], LG_BIT_MARK))   return FALSE ;

    // Success
    results->bits        = LG_BITS;
    results->value       = data;
    results->decode_type = LG;
    return TRUE;
}
#endif

//+=============================================================================
#if SEND_LG
void  sendLG (unsigned long data,  int nbits)
{
    // Set IR carrier frequency
    irsend_enableIROut(38);

    // Header
    irsend_mark(LG_HDR_MARK);
    irsend_space(LG_HDR_SPACE);
    irsend_mark(LG_BIT_MARK);

    // Data
    unsigned long  mask;
    for (mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
        if (data & mask) {
            irsend_space(LG_ONE_SPACE);
            irsend_mark(LG_BIT_MARK);
        } else {
            irsend_space(LG_ZERO_SPACE);
            irsend_mark(LG_BIT_MARK);
        }
    }
    irsend_space(0);  // Always end with the LED off
}
#endif

