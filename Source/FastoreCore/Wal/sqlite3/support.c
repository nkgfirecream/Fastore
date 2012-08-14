#include "sqliteInt.parts.h"

/*
 * The argument to this macro must be of type u32. On a little-endian
 * architecture, it returns the u32 value that results from interpreting
 * the 4 bytes as a big-endian value. On a big-endian architecture, it
 * returns the value that would be produced by intepreting the 4 bytes
 * of the input value as a little-endian integer.
 */
#define BYTESWAP32(x) ( \
    (((x)&0x000000FF)<<24) + (((x)&0x0000FF00)<<8)  \
  + (((x)&0x00FF0000)>>8)  + (((x)&0xFF000000)>>24) \
)

/*
 * Generate or extend an 8 byte checksum based on the data in 
 * array aByte[] and the initial values of aIn[0] and aIn[1] (or
 * initial values of 0 and 0 if aIn==NULL).
 *
 * The checksum is written back into aOut[] before returning.
 *
 * nByte must be a positive multiple of 8.
 */
static void walChecksumBytes(
  int nativeCksum, /* True for native byte-order, false for non-native */
  u8 *a,           /* Content to be checksummed */
  int nByte,       /* Bytes of content in a[].  Must be a multiple of 8. */
  const u32 *aIn,  /* Initial checksum value input */
  u32 *aOut        /* OUT: Final checksum value output */
)
{
  u32 s1, s2;
  u32 *aData = (u32 *)a;
  u32 *aEnd = (u32 *)&a[nByte];

  if( aIn ) {
    s1 = aIn[0];
    s2 = aIn[1];
  } else {
    s1 = s2 = 0;
  }

  assert( nByte>=8 );
  assert( (nByte&0x00000007)==0 );

  if( nativeCksum ) {
    do {
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
    } while( aData<aEnd );
  } else {

    do {
      s1 += BYTESWAP32(aData[0]) + s2;
      s2 += BYTESWAP32(aData[1]) + s1;
      aData += 2;
    } while( aData<aEnd );
  }

  aOut[0] = s1;
  aOut[1] = s2;
}

