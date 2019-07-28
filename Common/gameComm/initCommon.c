//
//
// initCommon.h - shared initialization functions
//-------------------------------------------------------------------------------------------------


#include "netio.h"
#include "varutils.h"
#include "fxinfo.h"
#include "entVarUpdate.h"
#include "timing.h"
#include "gameData\BodyPart.h"	// for bpReadBodyPartFiles()
#include "Npc.h"
#include "VillainDef.h"
#include "powers.h"
#include "error.h"
#include "fxinfo.h"
#include "FolderCache.h"
#include "fileutil.h"
#include "seqtype.h"
#include "cmdcommon.h"
#include "AccountData.h"

#if SERVER
	#include "Reward.h"
	#include "TeamReward.h"
	#include "entGameActions.h"
	#include "cmdserver.h"
#endif


#if CLIENT
	#include "initClient.h"
	#include "clientcomm.h"
	#include "uiDialog.h"
	#include "entclient.h"
	#include "player.h"
	#include "uiCostume.h"
	#include "fxbhvr.h"
	#include "fxfluid.h"
	#include "fxcapes.h"
	#include "cmdgame.h"
#endif


void cacheRelevantFolders()
{
	FolderCacheRequestTree(folder_cache, "Defs"); // If we're in dynamic mode, this will load this tree for faster file access
	FolderCacheRequestTree(folder_cache, "Menu");
	if (!quickload) {
		FolderCacheRequestTree(folder_cache, "player_library/animations/male");
		FolderCacheRequestTree(folder_cache, "player_library/animations/huge");
		FolderCacheRequestTree(folder_cache, "player_library/animations/fem");
		FolderCacheRequestTree(folder_cache, "player_library/animations/Vahzilok");
	}
}

//
//

void init_menus()
{
#ifdef SERVER
	extern int write_templates;
	if (!write_templates)
#endif
	{
		loadstart_printf("Loading FX info.. ");
			fxPreloadFxInfo();
		loadend_printf("done");

		loadstart_printf("Building FX string handles.. ");
			fxBuildFxStringHandles();
		loadend_printf("done");
	}

	loadstart_printf("Loading Loyalty Rewards.. ");
		accountLoyaltyRewardTreeLoad();
	loadend_printf("done");

#ifdef CLIENT
	loadstart_printf("Loading FX behaviors.. ");
		fxPreloadBhvrInfo();
	loadend_printf("done");

	loadstart_printf("Loading villain defs.. ");
		villainReadDefFiles();
	loadend_printf("done");

#ifdef NOVODEX_FLUIDS
	loadstart_printf("Loading FX fluids.. ");
	fxPreloadFluidInfo();
	loadend_printf("done");
#endif

	if (!STATE_STRUCT.nofx) {
		loadstart_printf("Loading FX capes.. ");
			fxPreloadCapeInfo();
		loadend_printf("done");
	}
#endif

	// FIXME!!!
	//	Move these two somewhere else.
	loadstart_printf("Loading body parts.. ");
		bpReadBodyPartFiles();
	loadend_printf("done");

#ifdef SERVER
	if (!server_state.levelEditor)
#endif

	loadstart_printf("Loading NPC defs.. ");
		npcReadDefFiles();
	loadend_printf("done");


#ifdef SERVER
	if (!server_state.tsr)
#endif
	{
		loadstart_printf("Loading ent_types.. ");
			seqTypeLoadFiles();
		loadend_printf("done");
	}


#ifdef SERVER
	if (!server_state.levelEditor)
	{
		loadstart_printf("Loading villain defs.. ");
			villainReadDefFiles();
		loadend_printf("done");

		loadstart_printf("Loading reward tables.. ");
			rewardReadDefFiles();
		loadend_printf("done");

		loadstart_printf("Loading team reward mods.. ");
			teamRewardReadDefFiles();
		loadend_printf("done");
	}
#endif

	// Everything should be loaded now, call the powers system back
	powerdict_FinalizeShared(&g_PowerDictionary);
}

