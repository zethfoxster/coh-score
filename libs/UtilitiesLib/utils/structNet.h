/***************************************************************************
*     Copyright (c) 2003-2004, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
* 
***************************************************************************/
#ifndef _STRUCTNET_H
#define _STRUCTNET_H

#include "stdtypes.h"
#include "textparser.h" // need FloatAccuracy enum from here

// MAK - these functions now work correctly with the full range of textparser structs

typedef struct Packet Packet;
typedef struct ParseTable ParseTable;

// lossy way of sending & receiving floats
int ParserPackFloat(float f, FloatAccuracy fa);
float ParserUnpackFloat(int i, FloatAccuracy fa);
int ParserFloatMinBits(FloatAccuracy fa);

// better packing for small deltas - promote to network lib?
void packDelta(Packet *pak, int delta);
int unpackDelta(Packet *pak);

void ParserSend(ParseTable* def, Packet* pak, void* oldVersion, void* newVersion, bool forcePackAllFields, bool allowDiffs, bool sendAbsoluteOverride, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
void ParserSendEmptyDiff(ParseTable* sd, Packet* pak, void* oldVersion, void* newVersion, bool forcePackAllFields, bool allowDiffs, bool sendAbsoluteOverride);
bool ParserRecv(ParseTable* sd, Packet* pak, void* data, void*** pktIds, bool sendAbsoluteOverride);

void ParserFreePktIds(ParseTable* sd, void *** pktIds);

// MAKTODO - replace these
void sdPackParseInfo(ParseTable* sd, Packet *pak);
ParseTable *sdUnpackParseInfo(ParseTable *supported, Packet *pak, bool needExactMatch);
void sdFreeParseInfo(ParseTable* fieldDefs);

// private
void sdPackStruct(ParseTable* tpi, Packet* pak, void* oldVersion, void* newVersion, bool forcePackAllFields, bool allowDiffs, StructTypeField iOptionFlagsToMatch, StructTypeField iOptionFlagsToExclude);
void sdUnpackStruct(ParseTable *fieldDefs, Packet *pak, void *data, void*** pktIds, bool allowDiffs);

#endif
