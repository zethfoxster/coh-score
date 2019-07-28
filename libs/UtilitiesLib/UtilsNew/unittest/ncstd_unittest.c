/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include <string.h>
#include "ncstd.h"
#include "ncunittest.h"
#include "ncHash.h"

void ncstd_unittest()
{
    {
        static int endian_check = 0x0D0C0B0A;
#define EB(i) (((char*)&endian_check)[i])
#ifdef __LITTLE_ENDIAN
        UTASSERT(__BYTE_ORDER == __LITTLE_ENDIAN);
        UTASSERT(EB(0) == 0xA &&  EB(1) == 0xB && EB(2) == 0xC && EB(3) == 0xD);
#elif defined(__BIG_ENDIAN)
        UTASSERT(__BYTE_ORDER == __BIG_ENDIAN && (EB(0) == 0xD && EB(1) == 0xC && EB(2) == 0xB && EB(3) == 0xA));
#endif
    }

    {
        char *s = strdup("asdf");
        free(s);
    }
    
}
