#include "LWC_common.h"
#include "gametypes.h"
#include "entity.h"
#include "entPlayer.h"
#include "clientcomm.h"
#include "comm_game.h"

#ifdef SERVER
#include "staticMapInfo.h"
#endif

LWC_STAGE LWC_GetRequiredDataStageForMap(const char *map_name)
{
	// This should match data/Defs/LightweightClient/lwc_stages.def
	if (stricmp(map_name, "N_City_00_01.txt") == 0)	// Neutral Tutorial
	{
		return LWC_STAGE_2;
	}
	else if ((stricmp(map_name, "City_01_01.txt") == 0) ||	// Atlas Park
	         (stricmp(map_name, "V_City_01_01.txt") == 0))	// Mercy Island
	{
		return LWC_STAGE_3;
	}

	return LWC_STAGE_LAST;
}

LWC_STAGE LWC_GetRequiredDataStageForMapID(int map_id)
{
	if (map_id == 41) // Neutral Tutorial
	{
		return LWC_STAGE_2;
	}

	// This should match data/Defs/LightweightClient/lwc_stages.def
	if (map_id == MAPID_ATLAS_PARK || map_id == MAPID_MERCY_ISLAND)
	{
		return LWC_STAGE_3;
	}

#ifdef SERVER
	// Assume any non-static maps are mission maps for Atlas Park or Mercy Island
	if (!staticMapInfoFind(map_id))
	{
		return LWC_STAGE_3;
	}
#endif

	return LWC_STAGE_4;
}


bool LWC_CheckMapReady(const char *map_name, LWC_STAGE current_stage, LWC_STAGE * out_required_stage)
{
	//printf("LWC_CheckMapReady(%s, %d)\n", map_name, current_stage);

	if (current_stage == LWC_STAGE_INVALID || current_stage >= LWC_STAGE_LAST)
	{
		return true;
	}
	else
	{
		LWC_STAGE required_stage = LWC_GetRequiredDataStageForMap(map_name);
		if (out_required_stage)
		{
			*out_required_stage = required_stage;
		}

		if (current_stage >= required_stage)
			return true;
	}

	return false;
}

bool LWC_CheckMapIDReady(int map_id, LWC_STAGE current_stage, LWC_STAGE * out_required_stage)
{
	//printf("LWC_CheckMapIDReady(%d, %d)\n", map_id, current_stage);

	if (current_stage == LWC_STAGE_INVALID || current_stage >= LWC_STAGE_LAST)
	{
		return true;
	}
	else
	{
		LWC_STAGE required_stage = LWC_GetRequiredDataStageForMapID(map_id);
		if (out_required_stage)
		{
			*out_required_stage = required_stage;
		}

		if (current_stage >= required_stage)
			return true;
	}

	return false;
}


bool LWC_CheckSGBaseReady(LWC_STAGE current_stage, LWC_STAGE * out_required_stage)
{
	if (current_stage == LWC_STAGE_INVALID || current_stage >= LWC_STAGE_LAST)
	{
		return true;
	}
	else
	{
		LWC_STAGE required_stage = LWC_STAGE_LAST;
		if (out_required_stage)
		{
			*out_required_stage = required_stage;
		}

		if (current_stage >= required_stage)
			return true;
	}

	return false;
}


LWC_STAGE LWC_GetCurrentStage(Entity *e)
{
	if (e && e->pl)
		return e->pl->lwc_stage;

	return LWC_STAGE_LAST;
}


#if CLIENT
void LWC_SendEntityStageToMapServer(Entity *e)
{
	if (e && e->pl)
	{
		Packet *pak = pktCreateEx(&comm_link, CLIENT_LWC_STAGE);
		pktSendBitsAuto(pak, e->db_id);
		pktSendBitsAuto(pak, e->id);
		pktSendBitsAuto(pak, e->pl->lwc_stage);
		pktSend(&pak,&comm_link);
		//printf("LWC_SendEntityStageToMapServer: %d\n", e->pl->lwc_stage);
	}
}
#endif

