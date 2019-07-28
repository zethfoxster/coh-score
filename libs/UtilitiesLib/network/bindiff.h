#ifndef _BINDIFF_H
#define _BINDIFF_H

#include "stdtypes.h"

int bindiffCreatePatch(U8 *orig,U32 orig_len,U8 *target,U32 target_len,U8 **output,int minMatchSize);
int bindiffApplyPatch(U8 *src,U8 *patchdata,U8 **target_p);

#endif
