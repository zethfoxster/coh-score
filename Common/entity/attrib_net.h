/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef ATTRIB_NET_H__
#define ATTRIB_NET_H__

#include "character_attribs.h"
#include "structNet.h"

typedef struct ParseTable ParseTable;
#define TokenizerParseInfo ParseTable

extern TokenizerParseInfo SendBasicCharacter[];
extern TokenizerParseInfo SendFullCharacter[];
extern TokenizerParseInfo SendCharacterToSelf[];

#endif /* #ifndef ATTRIB_NET_H__ */

/* End of File */

