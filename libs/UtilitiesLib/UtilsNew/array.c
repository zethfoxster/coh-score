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
#include "Array.h"
#include "ncHash.h"


#define TYPE_T void*
#define TYPE_FUNC_PREFIX ap
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define TYPE_T char
#define TYPE_FUNC_PREFIX achr
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define TYPE_T int
#define TYPE_FUNC_PREFIX aint
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX
