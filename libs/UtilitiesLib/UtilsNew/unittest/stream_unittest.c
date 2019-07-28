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
#include "stream.h"
#include "ncHash.h"
#include "Array.h"
#include "ncunittest.h"

void stream_unittest(void)
{
    TEST_START("stream_unittest");
    {
        char *s = 0;
        int is;
        int i,j;
        for( i = 0; i < 0xff; ++i ) 
        {
            for( j = 0; j < sizeof(int); ++j ) 
            {
                int pi;
                int v = (i+1) << j*8;
                int v2;
                pi = achr_size(&s);
                strm_write_int(&s,v);
                v2 = strm_read_int(&s,&pi);
                UTASSERT(v2 == v);
            }
        }

        is = 0;
        for( i = 0; i < 0xff; ++i ) 
        {
            for( j = 0; j < sizeof(int); ++j ) 
            {
                int pi = is;
                int v = (i+1) << j*8;
                int av;
                is = pi;
                av = strm_read_int(&s,&is); 
                UTASSERT(av == v);
            }
        }
        achr_destroy(&s);
    }
    TEST_END;
}
