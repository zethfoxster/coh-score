#ifndef SMF_INTERACT_H__
#define SMF_INTERACT_H__

#ifndef _STDTYPES_H
#include "stdtypes.h" // for bool
#endif

#include "smf_util.h"

void smf_CreateContextMenu(void);
bool smf_Navigate(char *pch);
void smf_Interact(SMFBlock *pSMFBlock, TextAttribs *pattrs, int iXBase, int iYBase, int iZBase, int width, int height, int (*callback)(char *pch));
bool smf_IsAnythingSelected(void);
void smf_setDebugDrawMode(int mode);

#endif //SMF_INTERACT_H__