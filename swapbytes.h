#ifndef SWAPBYTES_H
#define SWAPBYTES_H

//Swap a 16-bit integer
#define _SWAP16(x) ( \
  (((x) & 0x00FF) << 8) | \
  (((x) & 0xFF00) >> 8))
 
//Swap a 32-bit integer
#define _SWAP32(x) ( \
  (((x) & 0x000000FFUL) << 24) |  \
  (((x) & 0x0000FF00UL) << 8)  |  \
  (((x) & 0x00FF0000UL) >> 8)  |  \
  (((x) & 0xFF000000UL) >> 24))        

#define htonl(v) _SWAP32(v)
#define htons(v) _SWAP16(v)
#define ntohl(v) _SWAP32(v)
#define ntohs(v) _SWAP16(v)

#endif /* SWAPBYTES_H */

