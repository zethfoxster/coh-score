/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 ***************************************************************************/
#pragma once
#include "ncstd.h"
NCHEADER_BEGIN

// 'size' is always a multiple of 8*sizeof(unsigned)
typedef unsigned *Bits;

#define Bits_temp() Bits_temp_dbg(malloc_stack(1024), 1024 DBG_PARMS_INIT)
#define Bits_test(hbits, idx) Bits_test_dbg(hbits, idx DBG_PARMS_INIT)
#define Bits_set(hbits, idx) Bits_set_dbg(hbits, idx DBG_PARMS_INIT)
#define Bits_clear(hbits, idx) Bits_clear_dbg(hbits, idx DBG_PARMS_INIT)
#define Bits_toggle(hbits, idx) Bits_toggle_dbg(hbits, idx DBG_PARMS_INIT)
#define Bits_setvalue(hbits, idx, on) Bits_setvalue(hbits, idx, on DBG_PARMS_INIT)
#define Bits_setrun(hbits, idx, len) Bits_setrun_dbg(hbits, idx, len DBG_PARMS_INIT)
#define Bits_setbytes(hbits, mem, bytecount) Bits_setbytes_dbg(hbits, mem, bytecount DBG_PARMS_INIT)
#define Bits_clearall(hbits) Bits_clearall_dbg(hbits DBG_PARMS_INIT)
#define Bits_copy(hbits_dst, hbits_src) Bits_copy_dbg(hbits_dst, hbits_src DBG_PARMS_INIT)
#define Bits_and(hbits_dst, hbits_src) Bits_and_dbg(hbits_dst, hbits_src DBG_PARMS_INIT)
#define Bits_or(hbits_dst, hbits_src) Bits_or_dbg(hbits_dst, hbits_src DBG_PARMS_INIT)
#define Bits_xor(hbits_dst, hbits_src) Bits_xor_dbg(hbits_dst, hbits_src DBG_PARMS_INIT)
// #define Bits_not(hbits) Bits_not_dbg(hbits DBG_PARMS_INIT)
#define Bits_countset(hbits) Bits_countset_dbg(hbits DBG_PARMS_INIT)
#define Bits_highestset(hbits) Bits_highestset_dbg(hbits DBG_PARMS_INIT)
#define Bits_size(hbits) Bits_size_dbg(hbits DBG_PARMS_INIT)
#define Bits_destroy(hbits) Bits_destroy_dbg(hbits DBG_PARMS_INIT)

Bits Bits_temp_dbg(void *mem, size_t mem_size DBG_PARMS);
int Bits_test_dbg(const Bits *hbits, int idx DBG_PARMS); // returns value
int Bits_set_dbg(Bits *hbits, int idx DBG_PARMS); // returns 1 (new value)
int Bits_clear_dbg(Bits *hbits, int idx DBG_PARMS); // returns 0 (new value)
int Bits_toggle_dbg(Bits *hbits, int idx DBG_PARMS); // returns new value
int Bits_setvalue_dbg(Bits *hbits, int idx, int on DBG_PARMS); // returns new value
void Bits_setrun_dbg(Bits *hbits, int ixd, int len DBG_PARMS);
void Bits_setbytes_dbg(Bits *hbits, const void *mem, int bytecount DBG_PARMS);
void Bits_clearall_dbg(Bits *hbits DBG_PARMS);
void Bits_copy_dbg(Bits *hbits_dst, const Bits *hbits_src DBG_PARMS);
void Bits_and_dbg(Bits *hbits_dst, const Bits *hbits_src DBG_PARMS);
void Bits_or_dbg(Bits *hbits_dst, const Bits *hbits_src DBG_PARMS);
void Bits_xor_dbg(Bits *hbits_dst, const Bits *hbits_src DBG_PARMS);
// void Bits_not_dbg(Bits *hbits DBG_PARMS); // this doesn't fully fit the model
int Bits_countset_dbg(const Bits *hbits DBG_PARMS);
int Bits_highestset_dbg(const Bits *hbits DBG_PARMS);
int Bits_size_dbg(const Bits *hbits DBG_PARMS);
void Bits_destroy_dbg(Bits *hbits DBG_PARMS);

NCHEADER_END
