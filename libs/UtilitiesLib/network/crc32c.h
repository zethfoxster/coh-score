/**
* Implementation of CRC-32C (Castagnoli polynomrial)
*/

#ifndef CRC32C_H
#define CRC32C_H

C_DECLARATIONS_BEGIN

U32 Crc32c(U32 crc, const void * data, size_t bytes);
U32 Crc32cLowerStringHash(U32 hash, const char * s);

C_DECLARATIONS_END

#endif CRC32C_H

