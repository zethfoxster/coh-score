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
#include "Stream.h"
#include "ncHash.h"
#include "Array.h"

void strm_write_int_dbg(char* *handle, int a DBG_PARMS)
{
    if(!handle)
        return;
    // ab: kooky complicated scheme to save one byte in
    //     some cases :P
    // MSB :       range     : bytes transferred
    // 0   :       0-7F      : 1 byte
    // 10  :      80-BFFF    : 2 byte
    // 110 :    C000- (TBD)
    
    // simpler scheme:
    // leading byte is either:
    // - a count of follow-up bytes, if MSB is 1
    // - a byte value less than 128
    
    if(a < 128 && a >= 0)
    {
        *achr_push_dbg(handle DBG_PARMS_CALL) = (char)(a&0x7f);
        return;
    }
    else
    {
        int ta;
        int i;
        char nbytes;
        for( i = sizeof(int)-1; i >= 0; --i)
        {
            if(a & (0xff << i*8))
                break;
        }
        nbytes = (char)(i+1);
        assert(nbytes < 0x80);
        *achr_push_dbg(handle DBG_PARMS_CALL) = (char)(nbytes|0x80);

        ta = a;
        for( i = 0; i < nbytes; ++i ) 
        {
            *achr_push_dbg(handle DBG_PARMS_CALL) = (char)(ta&0xFF);
            ta>>=8;
        }
    }
}

int strm_read_int_dbg(char* *handle, int *hi DBG_PARMS)
{
    int i;
    int r = 0;
    char nbytes;
    U8 *data;
    line;caller_fname;
    if(!handle)
        return 0;    
    if(!hi)
        return 0;
    if(!INRANGE0(*hi,achr_size(handle)))
        return 0;
    data = (U8*)(*handle);
    nbytes = data[(*hi)];       // let's avoid sign extension
    (*hi)++;
    if(!(nbytes & 0x80))
        return (int)nbytes;
    nbytes &= 0x7F;
    if(nbytes > sizeof(int) || nbytes + (*hi) > achr_size(handle))
    {
        (*hi) = achr_size(handle); // push to end.
        return 0;
    }
    r = 0;
    for( i = 0; i < nbytes; ++i )
        r |= (data[(*hi) + i] << 8*i);
    (*hi)+=nbytes;
    return r;    
}

void strm_write_bytes_raw_dbg(char* *handle, char *bytes, int n DBG_PARMS)
{
    if(!handle || !bytes || n < 0)
        return;
    achr_append_dbg(handle,bytes,n DBG_PARMS_CALL);
}
