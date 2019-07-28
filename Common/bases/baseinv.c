/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "earray.h"
#include "MemoryPool.h"

#include "bases.h"
#include "basedata.h"

RoomDetail **g_ppDetailInv;

MP_DEFINE(RoomDetail);

/**********************************************************************func*
 * baseinv_Add
 *
 */
RoomDetail * baseinv_Add(const Detail *info)
{
	RoomDetail *pRoomDetail;
	MP_CREATE(RoomDetail, 20);

	pRoomDetail = MP_ALLOC(RoomDetail);
	pRoomDetail->id;
	pRoomDetail->info = info;

	eaPush(&g_ppDetailInv, pRoomDetail);

	return pRoomDetail;
}

/**********************************************************************func*
 * baseinv_RemoveIdx
 *
 */
void baseinv_RemoveIdx(int idx)
{
	RoomDetail *p = eaRemove(&g_ppDetailInv, idx);

	if(p)
	{
		MP_FREE(RoomDetail, p);
	}
}

/**********************************************************************func*
 * baseinv_Remove
 *
 */
void baseinv_Remove(RoomDetail *p)
{
	int i;
	int n = eaSize(&g_ppDetailInv);
	
	for(i=0; i<n; i++)
	{
		if(g_ppDetailInv[i] == p)
		{
			eaRemove(&g_ppDetailInv, i);
			break;
		}
	}

	MP_FREE(RoomDetail, p);
}

/* End of File */
