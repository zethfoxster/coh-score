#ifndef DOBBS_HASH_H
#define DOBBS_HASH_H

#if 0

typedef  unsigned long int  u4;   /* unsigned 4-byte type */
typedef  unsigned     char  u1;   /* unsigned 1-byte type */

u4 hashCalc(const void* k, u4	length, u4 initval);
u4 str_hash(const u1 *k);


#define	DEFAULT_HASH_SEED	( 123456 )
#define HASH(k,l)	hash( k, l, DEFAULT_HASH_SEED )
#define STR_HASH(k)	str_hash(k)

#endif

#endif