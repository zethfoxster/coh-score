/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef SMF_PARSE_H__
#define SMF_PARSE_H__

#include "smf_util.h"


int sm_ParseTagName(char *tagName, int tagNameLength, SMTagDef aTagDefs[]);

SMBlock *sm_Parse(char *pchOrig, SMFBlock *pSMFBlock, SMTagDef aTagDefs[], SMInitProc pfnInit, SMTextProc pfnText);
// Parses the given string using the standard conventions and the
// callback functions given. Returns a container block which must
// eventually be freed with SMDestroyBlock;

SMBlock *sm_ParseInto(SMBlock *pBlock, char *pchOrig, SMTagDef aTagDefs[], SMTextProc pfnText);
// Parses the given string using the standard conventions and the
// callback functions given. Uses the container block passed in.
// Use this function if special construction of the container block
// needs to be done before parsing.

SMBlock *smf_CreateAndParse(char *pch, SMFBlock *pSMFBlock);

#endif /* #ifndef SMF_PARSE_H__ */

/* End of File */

