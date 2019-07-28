/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include <assert.h>

#include "netio.h"
#include "netcomp.h"
#include "entity.h"
#include "character_base.h"
#include "character_net.h"
#include "player.h"
#include "timing.h"
#include "camera.h"

/**********************************************************************func*
 * entity_ActivatePowerAtLocationSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_ActivatePowerReceive)
 *
 */
void entity_ActivatePowerAtLocationSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow, Entity *ptarget, Vec3 vec)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBitsAuto(pak, ibuild);
	pktSendBitsPack(pak,  4, iset);
	pktSendBitsPack(pak,  4, ipow);
	pktSendBitsPack(pak, 16, ptarget->svr_idx);
	pktSendBitsPack(pak, 32, ptarget->id);
	pktSendF32( pak, camGetPlayerHeight() );
	pktSendVec3(pak, vec);
	pktSendVec3(pak, cam_info.cammat[3]);
}

/**********************************************************************func*
 * entity_ActivatePowerSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_ActivatePowerReceive)
 *
 */
void entity_ActivatePowerSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow, Entity *ptarget)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBitsAuto(pak, ibuild);
	pktSendBitsPack(pak, 4, iset);
	pktSendBitsPack(pak, 4, ipow);

	if(ptarget!=NULL)
	{
		pktSendBitsPack(pak, 16, ptarget->svr_idx);
		pktSendBitsPack(pak, 32, ptarget->id);
	}
	else
	{
		// used for assists
		pktSendBitsPack(pak, 16, 0);
		pktSendBitsPack(pak, 32, 0);
	}
}

/**********************************************************************func*
 * entity_SetDefaultPowerSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_SetDefaultPowerReceive)
 *
 */
void entity_SetDefaultPowerSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBitsAuto(pak, ibuild);
	pktSendBitsPack(pak, 4, iset);
	pktSendBitsPack(pak, 4, ipow);
}

/**********************************************************************func*
 * entity_ActivateInspirationSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_ActivateInspirationReceive)
 *
 */
void entity_ActivateInspirationSend(Entity *e, Packet *pak, int iCol, int iRow)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBitsPack(pak, 3, iCol);
	pktSendBitsPack(pak, 3, iRow);
}

/**********************************************************************func*
 * entity_EnterStanceSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_StanceReceive)
 *
 */
void entity_EnterStanceSend(Entity *e, Packet *pak, int ibuild, int iset, int ipow)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBits(pak, 1, 1);
	pktSendBitsAuto(pak, ibuild);
	pktSendBitsPack(pak, 4, iset);
	pktSendBitsPack(pak, 4, ipow);
}

/**********************************************************************func*
 * entity_ExitStanceSend
 *
 * Receive function in MapServer\entity\entity_power.c
 *    (entity_StanceReceive)
 *
 */
void entity_ExitStanceSend(Entity *e, Packet *pak)
{
	assert(e!=NULL);
	assert(pak!=NULL);

	pktSendBits(pak, 1, 0);
}

/**********************************************************************func*
 * entity_StanceReceive
 *
 */
void entity_StanceReceive(Packet *pak)
{
	int ibuild;
	int iset;
	int ipow;
	Power *ppow = NULL;
	Entity *e = playerPtr();
	extern U32 g_timeLastPower;

	g_timeLastPower = timerSecondsSince2000();

	assert(pak!=NULL);

	if(pktGetBits(pak, 1))
	{
		ibuild = pktGetBitsAuto(pak);
		iset = pktGetBitsPack(pak, 4);
		ipow = pktGetBitsPack(pak, 4);

		if(e)
			e->pchar->ppowStance = SafeGetPowerByIdx(e->pchar, ibuild, iset, ipow, 0);
	}
	else if(e)
	{
		e->pchar->ppowStance = NULL;
	}
}

/* End of File */
