#ifndef HASHFUNCTIONS_H
#define HASHFUNCTIONS_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "stdtypes.h"

C_DECLARATIONS_BEGIN

#define DEFAULT_HASH_SEED 0xfaceface

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;

ub4 burtlehash( const ub1 *k, ub4  length, ub4  initval);
ub4 burtlehash2( const ub4 *k, ub4  length, ub4  initval);
ub4 burtlehash3( const ub1 *k, ub4  length, ub4  initval);
ub4 burtlehash3StashDefault( const ub1 *k, ub4  length, ub4  initval);
ub4	hashString( const char* pcToHash, bool bCaseSensitive );
ub4	hashStringInsensitive( const char* pcToHash );
ub4 hashCalc( const void *k, ub4  length, ub4  initval);

C_DECLARATIONS_END

#endif