/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "ncunittest.h"

TestSuite g_testsuite;

void ncmem_unittest(void);

extern void nchash_unittest(void);
extern void ncstd_unittest();
extern int lookup3_unittest();
extern void ncarray_unittest(void);
extern void ncarray_unittest(void);
extern void ncarray_unittest(void);
    extern void str_unittest(void);
    extern void nchttp_unittest(void);
    extern void net_unittest(void);
    extern void stream_unittest(void);
void ncunittest_run(void)
{
    // tests
    {
        ncstd_unittest();
        ncmem_unittest();
        lookup3_unittest(); // string hash function
        nchash_unittest();
        ncarray_unittest();
        str_unittest();
        stream_unittest();
        net_unittest();
        nchttp_unittest();
    }
    printf("unit tests done.\n");
}
