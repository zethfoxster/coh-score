/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIGROWBIG_H
#define UIGROWBIG_H

#include "stdtypes.h"

//------------------------------------------------------------
//  a helper for making the 'over' icon grow bigger
//----------------------------------------------------------
typedef struct uiGrowBigItem
{ 
	F32 scale;
	void *over;
} uiGrowBigItem;

#define UIGROWBIGITEM_COUNT 4
typedef struct uiGrowBig
{
	uiGrowBigItem items[UIGROWBIGITEM_COUNT];
	F32 stepsize;
	F32 max;
} uiGrowBig;


void uigrowbig_Init(uiGrowBig *growbig, F32 max, U32 numSteps);
void uigrowbig_Update(uiGrowBig *growbig, void *over);
F32 uigrowbig_GetScale(uiGrowBig *growbig, void *over, int *elem);

#endif //UIGROWBIG_H
