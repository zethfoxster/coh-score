/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "uiGrowBig.h"
#include "utils.h"
#include "assert.h"

void uigrowbig_Init(uiGrowBig *growbig, F32 max, U32 numSteps)
{
	if( verify( growbig ))
	{
		ZeroStruct(growbig);

		// only need to init first one
		growbig->items[0].scale = 1.f;
		growbig->max = max;
		growbig->stepsize = max/numSteps;
	}
}

void uigrowbig_Update(uiGrowBig *growbig, void *over)
{
	if( verify( growbig ))
	{
		int i;
		
		if( growbig->items[0].over != over )
		{
			// move everything down
			CopyStructs(growbig->items+1,growbig->items, ARRAY_SIZE( growbig->items ) - 1);
			growbig->items[0].scale = 1.f;
			growbig->items[0].over = over;
		}

		// update
		for( i = 0; i < ARRAY_SIZE( growbig->items ); ++i ) 
		{
			growbig->items[i].scale = CLAMPF32(growbig->items[i].scale + (i == 0 ? growbig->stepsize : -growbig->stepsize), 1.f, growbig->max);
		}
	}
}

F32 uigrowbig_GetScale(uiGrowBig *growbig, void *over, int *elem)
{
	F32 res = 1.f;

	if( verify( growbig ))
	{
		int i;
		for( i = 0; i < ARRAY_SIZE( growbig->items ); ++i ) 
		{
			if( growbig->items[i].over == over )
			{
				res = growbig->items[i].scale;

				if( *elem )
				{
					*elem = i;
				}

				break;
			}
		}
	}
	// --------------------
	// finally

	return res;
}

