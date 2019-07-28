/*----------------------------------------------------------------------
    This Software and Related Documentation are Proprietary to Ageia
    Technologies, Inc.

    Copyright 2005 Ageia Technologies, Inc. St. Louis, MO
    Unpublished -
    All Rights Reserved Under the Copyright Laws of the United States.

    Restricted Rights Legend:  Use, Duplication, or Disclosure by
    the Government is Subject to Restrictions as Set Forth in
    Paragraph (c)(1)(ii) of the Rights in Technical Data and
    Computer Software Clause at DFARS 252.227-7013.  Ageia
    Technologies Inc.
-----------------------------------------------------------------------*/

#ifndef PORTABLE_FLOAT_H
#define PORTABLE_FLOAT_H 1

#include "portableInt.h"

/*---------------------------------------------------------------------*/
/**
 *  Portable type definitions ONLY -- No methods or function prototypes!
 */

/*---------------------------------------------------------------------*/
/**
 *  Portable type definitions for floating point numbers...
 */


typedef float    f32;
typedef double   f64;

typedef struct
{
	f32 *  elt;
	u32    numElts;
	u32    maxElts;
}
f32vector;

typedef struct
{
	f64 *  elt;
	u32    numElts;
	u32    maxElts;
}
f64vector;

/*---------------------------------------------------------------------*/
/**
 *  Portable type definitions for floating point vectors...
 */

typedef struct
{
    f32    x;
    f32    y;
}
f32v2;

typedef struct
{
    f32v2 *  elt;
	u32      numElts;
	u32      maxElts;
}
f32v2vector;

typedef struct
{
    f32    x;
    f32    y;
    f32    z;
}
f32v3;

typedef struct
{
    f32v3 *  elt;
	u32      numElts;
	u32      maxElts;
}
f32v3vector;

typedef struct
{
    f32    x;
    f32    y;
    f32    z;
    f32    w;
}
f32v4;

typedef struct
{
    f32v4 *  elt;
	u32      numElts;
	u32      maxElts;
}
f32v4vector;

/*---------------------------------------------------------------------*/
/**
 *  Portable type definitions for floating point matrices...
 */



typedef struct
{
    f32v3        right;
	f32v3        up;
	f32v3        at;
}
f32m33;

typedef struct
{
    f32m33 *  elt;
	u32       numElts;
	u32       maxElts;
}
f32m33vector;

typedef struct
{
	f32m33		M;
	f32v3		t;
}
f32m34;

typedef struct
{
    f32v4      right;
	f32v4      up;
	f32v4      at;
	f32v4      pos;
}
f32m44;

typedef struct
{
    f32m44 *  elt;
	u32       numElts;
	u32       maxElts;
}
f32m44vector;

#endif /* PORTABLE_FLOAT_H */
