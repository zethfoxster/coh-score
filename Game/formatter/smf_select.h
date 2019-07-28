#ifndef SMF_SELECT_H__
#define SMF_SELECT_H__

#ifndef _STDTYPES_H
#include "stdtypes.h" // for bool
#endif

#include "smf_util.h"

void smf_ResetSelectedBlocks(SMBlock *pBlock);
void smf_RebuildPartialSelectionFromSMFBlock(SMFBlock *pSMFBlock, int x, int y, int z);
void smf_Select(SMBlock *pBlock, TextAttribs *pattrs, int iXBase, int iYBase, int iZBase);
char *smf_GetSelectionString(void);
void smf_HandleSelectPerFrame(void);

#endif //SMF_SELECT_H__