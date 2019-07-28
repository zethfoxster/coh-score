/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef STREAM_H
#define STREAM_H

#include "ncstd.h"

NCHEADER_BEGIN

typedef struct Stream
{
    U8 *data;
    int i;
} Stream;


#define strm_write_int(handle, a ) strm_write_int_dbg(handle, a  DBG_PARMS_INIT)
#define strm_read_int(handle,hi) strm_read_int_dbg(handle, hi  DBG_PARMS_INIT)
#define strm_write_bytes_raw(handle, hi, bytes, n ) strm_write_bytes_raw_dbg(handle, hi, bytes, n  DBG_PARMS_INIT)

void strm_write_int_dbg(char* *handle, int a DBG_PARMS);
int strm_read_int_dbg(char* *handle, int *hi DBG_PARMS);

void strm_write_bytes_raw_dbg(char* *handle, char *bytes, int n DBG_PARMS);

NCHEADER_END
#endif //STREAM_H
