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
#include "ncHash.h"

extern "C" void ncmem_unittest(void)
{
    TEST_START("strdup");
    {
        int i;
        char *strs[1024];
        for( i = 0; i < ARRAY_SIZE(strs); ++i ) 
        {
            char tmp[128];
            sprintf_s(ASTR(tmp),"%i",i);
            strs[i] = _strdup(tmp);
            free(strs[i]);
        }
    }
    TEST_END;
}
