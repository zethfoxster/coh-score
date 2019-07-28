/***************************************************************************
*     Copyright (c) 2003-2005, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
*
* Module Description:
*   Allows for keeping a running average and (to add later) a recent history
* 
***************************************************************************/
#ifndef STRUCTHIST_H
#define STRUCTHIST_H

#include "stdtypes.h"
#include "textparser.h"

typedef struct StructHistImp *StructHist;

StructHist createStructHist(ParseTable *tpi, char *folder);
void destroyStructHist(StructHist sh);

void shSample(StructHist sh, void *elem, U32 timestamp);
void shReset(StructHist sh);
void shDump(StructHist sh, const char *logdir);

typedef struct StructHistCollectionImp *StructHistCollection;

StructHistCollection createStructHistCollection(const char *logDir);
void destroyStructHistCollection(StructHistCollection shc);

void shcRemove(StructHistCollection shc, void *elem, U32 timestamp);
void shcUpdate(StructHistCollection *pshc, void *elem, ParseTable *tpi, U32 timestamp, char *folder); // Should be called immediately *before* changing a value
void shcDump(StructHistCollection shc, U32 timestamp);

typedef enum StructOp {
	STRUCTOP_ADD,
	STRUCTOP_MIN,
	STRUCTOP_MAX,
	STRUCTOP_DIV,
	STRUCTOP_MUL,
	STRUCTOP_LERP,	// For LERP, call shDoOperationSetFloat() and pass two structures to shDoOperation; 0.0 means 100% dest
} StructOp;

#define OPERAND_INT ((void*)(intptr_t)-1)
#define OPERAND_FLOAT ((void*)(intptr_t)-2)

void shDoOperationSetInt(int i);
void shDoOperationSetFloat(F32 f);
void shDoOperation(StructOp op, ParseTable *tpi, void *dest, const void *operand);

void shSetOperationSilentlyIgnoreEArrays(bool ignore); // Otherwise it provides a warning about passing EArrays

#endif