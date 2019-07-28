// ============================================================
// META generated file. do not edit!
// Generated: Mon Apr 19 23:43:22 2010
// ============================================================

#include "missionserver_meta.h"

#include "netio.h"

// FIXME: thread-safety

#include "UtilsNew/Array.h"
#include "UtilsNew/lock.h"

#if PROJECT_MISSIONSERVER
void missionserver_ForceLoadDb(int force);
void missionserver_DumpStats(int force);
#endif // PROJECT_MISSIONSERVER

#if !CLIENT
void missionserver_setLiveConfig(char *var, char *val);
void missionserver_printArcInfo(int arcid);
void missionserver_banUser(int auth_id, char *auth_region_str, int days);
void missionserver_removecomplaint(int arcid, int complainer_id, char *complainer_region_str);
void missionserver_setCirculationStatus(int accesslevel, int arcid, int auth_id, int circulate);

void missionserver_setLiveConfig_receivepacket(Packet *pak_in)
{
	char *var = pktGetStringTemp(pak_in);
	char *val = pktGetStringTemp(pak_in);
	missionserver_setLiveConfig(var, val);
}

void missionserver_unpublishArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	int accesslevel = pktGetBitsAuto(pak_in);
	missionserver_unpublishArc(arcid, auth_id, accesslevel);
}

void missionserver_updateLevelRange_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int low = pktGetBitsAuto(pak_in);
	int high = pktGetBitsAuto(pak_in);
	missionserver_updateLevelRange(arcid, low, high);
}

void missionserver_invalidateArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int unableToPlay = pktGetBitsAuto(pak_in);
	char *error = pktGetStringTemp(pak_in);
	missionserver_invalidateArc(arcid, unableToPlay, error);
}

void missionserver_restoreArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	missionserver_restoreArc(arcid);
}

void missionserver_changeHonors_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int ihonors = pktGetBitsAuto(pak_in);
	missionserver_changeHonors(arcid, ihonors);
}

void missionserver_banArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	missionserver_banArc(arcid);
}

void missionserver_unbannableArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	missionserver_unbannableArc(arcid);
}

void missionserver_printArcInfo_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	missionserver_printArcInfo(arcid);
}

void missionserver_grantTickets_receivepacket(Packet *pak_in)
{
	int tickets = pktGetBitsAuto(pak_in);
	int db_id = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	missionserver_grantTickets(tickets, db_id, auth_id);
}

void missionserver_grantOverflowTickets_receivepacket(Packet *pak_in)
{
	int tickets = pktGetBitsAuto(pak_in);
	int db_id = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	missionserver_grantOverflowTickets(tickets, db_id, auth_id);
}

void missionserver_setPaidPublishSlots_receivepacket(Packet *pak_in)
{
	int paid_slots = pktGetBitsAuto(pak_in);
	int db_id = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	missionserver_setPaidPublishSlots(paid_slots, db_id, auth_id);
}

void missionserver_recordArcCompleted_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int authid = pktGetBitsAuto(pak_in);
	missionserver_recordArcCompleted(arcid, authid);
}

void missionserver_recordArcStarted_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int authid = pktGetBitsAuto(pak_in);
	missionserver_recordArcStarted(arcid, authid);
}

void missionserver_printUserInfo_receivepacket(Packet *pak_in)
{
	int auth_id = pktGetBitsAuto(pak_in);
	char *auth_region_str = pktGetStringTemp(pak_in);
	missionserver_printUserInfo(auth_id, auth_region_str);
}

void missionserver_banUser_receivepacket(Packet *pak_in)
{
	int auth_id = pktGetBitsAuto(pak_in);
	char *auth_region_str = pktGetStringTemp(pak_in);
	int days = pktGetBitsAuto(pak_in);
	missionserver_banUser(auth_id, auth_region_str, days);
}

void missionserver_setBestArcStars_receivepacket(Packet *pak_in)
{
	int stars = pktGetBitsAuto(pak_in);
	int db_id = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	missionserver_setBestArcStars(stars, db_id, auth_id);
}

void missionserver_reportArc_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	int type = pktGetBitsAuto(pak_in);
	char *comment = pktGetStringTemp(pak_in);
	char *globalName = pktGetStringTemp(pak_in);
	missionserver_reportArc(arcid, auth_id, type, comment, globalName);
}

void missionserver_removecomplaint_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int complainer_id = pktGetBitsAuto(pak_in);
	char *complainer_region_str = pktGetStringTemp(pak_in);
	missionserver_removecomplaint(arcid, complainer_id, complainer_region_str);
}

void missionserver_comment_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	char *comment = pktGetStringTemp(pak_in);
	missionserver_comment(arcid, auth_id, comment);
}

void missionserver_setKeywordsForArc_receivepacket(Packet *pak_in)
{
	int accesslevel = pktGetBitsAuto(pak_in);
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	int keywords = pktGetBitsAuto(pak_in);
	missionserver_setKeywordsForArc(accesslevel, arcid, auth_id, keywords);
}

void missionserver_setCirculationStatus_receivepacket(Packet *pak_in)
{
	int accesslevel = pktGetBitsAuto(pak_in);
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	int circulate = pktGetBitsAuto(pak_in);
	missionserver_setCirculationStatus(accesslevel, arcid, auth_id, circulate);
}

void missionserver_voteForArc_receivepacket(Packet *pak_in)
{
	int accesslevel = pktGetBitsAuto(pak_in);
	int arcid = pktGetBitsAuto(pak_in);
	int auth_id = pktGetBitsAuto(pak_in);
	int ivote = pktGetBitsAuto(pak_in);
	missionserver_voteForArc(accesslevel, arcid, auth_id, ivote);
}

void missionserver_setGuestBio_receivepacket(Packet *pak_in)
{
	int arcid = pktGetBitsAuto(pak_in);
	char *author = pktGetStringTemp(pak_in);
	char *bio = pktGetStringTemp(pak_in);
	char *image = pktGetStringTemp(pak_in);
	missionserver_setGuestBio(arcid, author, bio, image);
}
#endif // !CLIENT

void missionserver_meta_register(void)
{
	static int registered = 0;
	static LazyLock lock = {0};

	ObjectInfo *object = NULL;
	TypeParameter *parameter = NULL;

	if(registered)
		return;

	lazyLock(&lock);
	if(registered)
	{
		lazyUnlock(&lock);
		return;
	}

#if PROJECT_MISSIONSERVER
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_ForceLoadDb";
	object->typeinfo.type = TYPE_FUNCTION;
	object->console_reqparams = 1;
	object->location = missionserver_ForceLoadDb;
	object->filename = "MissionServer.c";
	object->fileline = 285;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "force";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_COMMANDLINE, object, "forceLoadDb");
	#endif
#endif // PROJECT_MISSIONSERVER

#if PROJECT_MISSIONSERVER
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_DumpStats";
	object->typeinfo.type = TYPE_FUNCTION;
	object->console_reqparams = 1;
	object->location = missionserver_DumpStats;
	object->filename = "MissionServer.c";
	object->fileline = 291;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "force";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_COMMANDLINE, object, "dumpStats");
	#endif
#endif // PROJECT_MISSIONSERVER

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setLiveConfig";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 3;
	object->console_desc = "modifies the missionserver's live config. call with no parameters for more info.";
	object->console_reqparams = 0;
	object->location = missionserver_setLiveConfig;
	object->filename = "MissionServer.c";
	object->fileline = 646;
	object->receivepacket = missionserver_setLiveConfig_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "var";
	parameter->typeinfo.type = TYPE_STRING;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "val";
	parameter->typeinfo.type = TYPE_STRING;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "set");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_set");
	#endif
	#if DBSERVER || PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setLiveConfig");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_unpublishArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_unpublishArc;
	object->filename = "MissionServer.c";
	object->fileline = 700;
	object->receivepacket = missionserver_unpublishArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "accesslevel";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_unpublishArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_updateLevelRange";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_updateLevelRange;
	object->filename = "MissionServer.c";
	object->fileline = 752;
	object->receivepacket = missionserver_updateLevelRange_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "low";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "high";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_updateLevelRange");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_invalidateArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 9;
	object->console_desc = "arc_invalidate <arcid> sends an invalidation command to the mission server";
	object->console_reqparams = 3;
	object->location = missionserver_invalidateArc;
	object->filename = "MissionServer.c";
	object->fileline = 772;
	object->receivepacket = missionserver_invalidateArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "unableToPlay";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "error";
	parameter->typeinfo.type = TYPE_STRING;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_invalidate");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_invalidate");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_invalidateArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_restoreArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Restores unpublished arc <arcid>";
	object->console_reqparams = 1;
	object->location = missionserver_restoreArc;
	object->filename = "MissionServer.c";
	object->fileline = 854;
	object->receivepacket = missionserver_restoreArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_restore");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_restore");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_restoreArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_changeHonors";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_reqparams = 2;
	object->location = missionserver_changeHonors;
	object->filename = "MissionServer.c";
	object->fileline = 909;
	object->receivepacket = missionserver_changeHonors_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "ihonors";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_changeHonors");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_banArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Ban arc <arcid>";
	object->console_reqparams = 1;
	object->location = missionserver_banArc;
	object->filename = "MissionServer.c";
	object->fileline = 1017;
	object->receivepacket = missionserver_banArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_ban");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_ban");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_banArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_unbannableArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Make arc <arcid> unbannable";
	object->console_reqparams = 1;
	object->location = missionserver_unbannableArc;
	object->filename = "MissionServer.c";
	object->fileline = 1023;
	object->receivepacket = missionserver_unbannableArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_approve");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_approve");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_unbannableArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_printArcInfo";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Get exhaustive info on <arcid>";
	object->console_reqparams = 1;
	object->location = missionserver_printArcInfo;
	object->filename = "MissionServer.c";
	object->fileline = 1029;
	object->receivepacket = missionserver_printArcInfo_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_info");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_info");
	#endif
	#if DBSERVER || PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_printArcInfo");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_grantTickets";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_grantTickets;
	object->filename = "MissionServer.c";
	object->fileline = 1310;
	object->receivepacket = missionserver_grantTickets_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "tickets";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "db_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_grantTickets");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_grantOverflowTickets";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_grantOverflowTickets;
	object->filename = "MissionServer.c";
	object->fileline = 1335;
	object->receivepacket = missionserver_grantOverflowTickets_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "tickets";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "db_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_grantOverflowTickets");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setPaidPublishSlots";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_setPaidPublishSlots;
	object->filename = "MissionServer.c";
	object->fileline = 1370;
	object->receivepacket = missionserver_setPaidPublishSlots_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "paid_slots";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "db_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setPaidPublishSlots");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_recordArcCompleted";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 2;
	object->location = missionserver_recordArcCompleted;
	object->filename = "MissionServer.c";
	object->fileline = 1520;
	object->receivepacket = missionserver_recordArcCompleted_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "authid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_recordArcCompleted");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_recordArcStarted";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 2;
	object->location = missionserver_recordArcStarted;
	object->filename = "MissionServer.c";
	object->fileline = 1621;
	object->receivepacket = missionserver_recordArcStarted_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "authid";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_recordArcStarted");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_printUserInfo";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Get exhaustive info on <authid> <authregion>";
	object->console_reqparams = 2;
	object->location = missionserver_printUserInfo;
	object->filename = "MissionServer.c";
	object->fileline = 1652;
	object->receivepacket = missionserver_printUserInfo_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_region_str";
	parameter->typeinfo.type = TYPE_STRING;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "user_info");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_user_info");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_printUserInfo");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_banUser";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Ban user <authid> <authregion> for <days> days (0 days to unban)";
	object->console_reqparams = 3;
	object->location = missionserver_banUser;
	object->filename = "MissionServer.c";
	object->fileline = 1708;
	object->receivepacket = missionserver_banUser_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_region_str";
	parameter->typeinfo.type = TYPE_STRING;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "days";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "user_ban");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_user_ban");
	#endif
	#if DBSERVER || PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_banUser");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setBestArcStars";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_setBestArcStars;
	object->filename = "MissionServer.c";
	object->fileline = 1741;
	object->receivepacket = missionserver_setBestArcStars_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "stars";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "db_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setBestArcStars");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_reportArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 5;
	object->location = missionserver_reportArc;
	object->filename = "MissionServer.c";
	object->fileline = 1767;
	object->receivepacket = missionserver_reportArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "type";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "comment";
	parameter->typeinfo.type = TYPE_STRING;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "globalName";
	parameter->typeinfo.type = TYPE_STRING;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_reportArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_removecomplaint";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 1;
	object->console_desc = "Remove complaint on <arcid> made by <authid> <authregion>";
	object->console_reqparams = 3;
	object->location = missionserver_removecomplaint;
	object->filename = "MissionServer.c";
	object->fileline = 1856;
	object->receivepacket = missionserver_removecomplaint_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "complainer_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "complainer_region_str";
	parameter->typeinfo.type = TYPE_STRING;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "removecomplaint");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_removecomplaint");
	#endif
	#if DBSERVER || PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_removecomplaint");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_comment";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 3;
	object->location = missionserver_comment;
	object->filename = "MissionServer.c";
	object->fileline = 1887;
	object->receivepacket = missionserver_comment_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "comment";
	parameter->typeinfo.type = TYPE_STRING;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_comment");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setKeywordsForArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 4;
	object->location = missionserver_setKeywordsForArc;
	object->filename = "MissionServer.c";
	object->fileline = 1957;
	object->receivepacket = missionserver_setKeywordsForArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "accesslevel";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "keywords";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setKeywordsForArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setCirculationStatus";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 10;
	object->console_desc = "Set the circulation status of arc <arcid> made by <authid> <authregion> to <on/off>";
	object->console_reqparams = 4;
	object->location = missionserver_setCirculationStatus;
	object->filename = "MissionServer.c";
	object->fileline = 2020;
	object->receivepacket = missionserver_setCirculationStatus_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "accesslevel";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "circulate";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "setCirculationStatus");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_setCirculationStatus");
	#endif
	#if DBSERVER || PROJECT_MISSIONSERVER
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setCirculationStatus");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_voteForArc";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 11;
	object->console_reqparams = 4;
	object->location = missionserver_voteForArc;
	object->filename = "MissionServer.c";
	object->fileline = 2105;
	object->receivepacket = missionserver_voteForArc_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "accesslevel";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "auth_id";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "ivote";
	parameter->typeinfo.type = TYPE_INTEGER;

	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_voteForArc");
	#endif
#endif // !CLIENT

#if !CLIENT
	object = calloc(1, sizeof(*object));
	object->name = "missionserver_setGuestBio";
	object->typeinfo.type = TYPE_FUNCTION;
	object->accesslevel = 3;
	object->console_desc = "arc_author_bio <arcid> <author display name> <biographical info> <image name>";
	object->console_reqparams = 4;
	object->location = missionserver_setGuestBio;
	object->filename = "MissionServer.c";
	object->fileline = 2215;
	object->receivepacket = missionserver_setGuestBio_receivepacket;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "arcid";
	parameter->typeinfo.type = TYPE_INTEGER;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "author";
	parameter->typeinfo.type = TYPE_STRING;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "bio";
	parameter->typeinfo.type = TYPE_STRING;

	parameter = as_push(&object->typeinfo.function_parameters);
	parameter->name = "image";
	parameter->typeinfo.type = TYPE_STRING;

	#if SERVER
		meta_registerCall(CALLTYPE_CONSOLE, object, "arc_author_bio");
		meta_registerCall(CALLTYPE_CONSOLE, object, "missionserver_arc_author_bio");
	#endif
	#if !CLIENT
		meta_registerCall(CALLTYPE_REMOTE, object, "missionserver_setGuestBio");
	#endif
#endif // !CLIENT

	registered = 1;
	lazyUnlock(&lock);
}

#if DBSERVER
#include "comm_backend.h"
#include "missionservercomm.h"

void missionserver_setLiveConfig(char *var, char *val)
{
	// put a breakpoint in MissionServer.c around line 646
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setLiveConfig");
		pktSendStringAligned(pak_out, var);
		pktSendStringAligned(pak_out, val);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_unpublishArc(int arcid, int auth_id, int accesslevel)
{
	// put a breakpoint in MissionServer.c around line 700
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_unpublishArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendBitsAuto(pak_out, accesslevel);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_updateLevelRange(int arcid, int low, int high)
{
	// put a breakpoint in MissionServer.c around line 752
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_updateLevelRange");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, low);
		pktSendBitsAuto(pak_out, high);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_invalidateArc(int arcid, int unableToPlay, char *error)
{
	// put a breakpoint in MissionServer.c around line 772
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_invalidateArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, unableToPlay);
		pktSendStringAligned(pak_out, error);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_restoreArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 854
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_restoreArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_changeHonors(int arcid, int ihonors)
{
	// put a breakpoint in MissionServer.c around line 909
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_changeHonors");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, ihonors);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_banArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1017
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_banArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_unbannableArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1023
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_unbannableArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_printArcInfo(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1029
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_printArcInfo");
		pktSendBitsAuto(pak_out, arcid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_grantTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1310
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_grantTickets");
		pktSendBitsAuto(pak_out, tickets);
		pktSendBitsAuto(pak_out, db_id);
		pktSendBitsAuto(pak_out, auth_id);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_grantOverflowTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1335
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_grantOverflowTickets");
		pktSendBitsAuto(pak_out, tickets);
		pktSendBitsAuto(pak_out, db_id);
		pktSendBitsAuto(pak_out, auth_id);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_setPaidPublishSlots(int paid_slots, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1370
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setPaidPublishSlots");
		pktSendBitsAuto(pak_out, paid_slots);
		pktSendBitsAuto(pak_out, db_id);
		pktSendBitsAuto(pak_out, auth_id);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_recordArcCompleted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1520
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_recordArcCompleted");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, authid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_recordArcStarted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1621
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_recordArcStarted");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, authid);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_printUserInfo(int auth_id, char *auth_region_str)
{
	// put a breakpoint in MissionServer.c around line 1652
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_printUserInfo");
		pktSendBitsAuto(pak_out, auth_id);
		pktSendStringAligned(pak_out, auth_region_str);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_banUser(int auth_id, char *auth_region_str, int days)
{
	// put a breakpoint in MissionServer.c around line 1708
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_banUser");
		pktSendBitsAuto(pak_out, auth_id);
		pktSendStringAligned(pak_out, auth_region_str);
		pktSendBitsAuto(pak_out, days);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_setBestArcStars(int stars, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1741
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setBestArcStars");
		pktSendBitsAuto(pak_out, stars);
		pktSendBitsAuto(pak_out, db_id);
		pktSendBitsAuto(pak_out, auth_id);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_reportArc(int arcid, int auth_id, int type, const char *comment, const char *globalName)
{
	// put a breakpoint in MissionServer.c around line 1767
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_reportArc");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendBitsAuto(pak_out, type);
		pktSendStringAligned(pak_out, comment);
		pktSendStringAligned(pak_out, globalName);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_removecomplaint(int arcid, int complainer_id, char *complainer_region_str)
{
	// put a breakpoint in MissionServer.c around line 1856
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_removecomplaint");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, complainer_id);
		pktSendStringAligned(pak_out, complainer_region_str);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_comment(int arcid, int auth_id, const char *comment)
{
	// put a breakpoint in MissionServer.c around line 1887
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_comment");
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendStringAligned(pak_out, comment);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_setKeywordsForArc(int accesslevel, int arcid, int auth_id, int keywords)
{
	// put a breakpoint in MissionServer.c around line 1957
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setKeywordsForArc");
		pktSendBitsAuto(pak_out, accesslevel);
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendBitsAuto(pak_out, keywords);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_setCirculationStatus(int accesslevel, int arcid, int auth_id, int circulate)
{
	// put a breakpoint in MissionServer.c around line 2020
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setCirculationStatus");
		pktSendBitsAuto(pak_out, accesslevel);
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendBitsAuto(pak_out, circulate);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_voteForArc(int accesslevel, int arcid, int auth_id, int ivote)
{
	// put a breakpoint in MissionServer.c around line 2105
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_voteForArc");
		pktSendBitsAuto(pak_out, accesslevel);
		pktSendBitsAuto(pak_out, arcid);
		pktSendBitsAuto(pak_out, auth_id);
		pktSendBitsAuto(pak_out, ivote);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

void missionserver_setGuestBio(int arcid, char *author, char *bio, char *image)
{
	// put a breakpoint in MissionServer.c around line 2215
	NetLink *link = missionserver_getLink();
	if(link)
	{
		Packet *pak_out = pktCreateEx(link, MISSIONSERVER_CLIENT_COMMAND);
		meta_pktSendRoute(pak_out);
		pktSendStringAligned(pak_out, "missionserver_setGuestBio");
		pktSendBitsAuto(pak_out, arcid);
		pktSendStringAligned(pak_out, author);
		pktSendStringAligned(pak_out, bio);
		pktSendStringAligned(pak_out, image);
		pktSend(&pak_out, link);
	}
	else
	{
		// reply with failure
		meta_printf("MissionServerNoConnection");
	}
}

#endif // DBSERVER

#if SERVER
#include "comm_backend.h"
#include "dbcomm.h"

void missionserver_setLiveConfig(char *var, char *val)
{
	// put a breakpoint in MissionServer.c around line 646
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setLiveConfig");
	pktSendStringAligned(pak_out, var);
	pktSendStringAligned(pak_out, val);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_unpublishArc(int arcid, int auth_id, int accesslevel)
{
	// put a breakpoint in MissionServer.c around line 700
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_unpublishArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, accesslevel);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_updateLevelRange(int arcid, int low, int high)
{
	// put a breakpoint in MissionServer.c around line 752
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_updateLevelRange");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, low);
	pktSendBitsAuto(pak_out, high);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_invalidateArc(int arcid, int unableToPlay, char *error)
{
	// put a breakpoint in MissionServer.c around line 772
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_invalidateArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, unableToPlay);
	pktSendStringAligned(pak_out, error);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_restoreArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 854
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_restoreArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_changeHonors(int arcid, int ihonors)
{
	// put a breakpoint in MissionServer.c around line 909
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_changeHonors");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, ihonors);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_banArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1017
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_banArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_unbannableArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1023
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_unbannableArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_printArcInfo(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1029
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_printArcInfo");
	pktSendBitsAuto(pak_out, arcid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_grantTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1310
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_grantTickets");
	pktSendBitsAuto(pak_out, tickets);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_grantOverflowTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1335
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_grantOverflowTickets");
	pktSendBitsAuto(pak_out, tickets);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_setPaidPublishSlots(int paid_slots, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1370
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setPaidPublishSlots");
	pktSendBitsAuto(pak_out, paid_slots);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_recordArcCompleted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1520
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_recordArcCompleted");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, authid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_recordArcStarted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1621
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_recordArcStarted");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, authid);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_printUserInfo(int auth_id, char *auth_region_str)
{
	// put a breakpoint in MissionServer.c around line 1652
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_printUserInfo");
	pktSendBitsAuto(pak_out, auth_id);
	pktSendStringAligned(pak_out, auth_region_str);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_banUser(int auth_id, char *auth_region_str, int days)
{
	// put a breakpoint in MissionServer.c around line 1708
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_banUser");
	pktSendBitsAuto(pak_out, auth_id);
	pktSendStringAligned(pak_out, auth_region_str);
	pktSendBitsAuto(pak_out, days);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_setBestArcStars(int stars, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1741
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setBestArcStars");
	pktSendBitsAuto(pak_out, stars);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_reportArc(int arcid, int auth_id, int type, const char *comment, const char *globalName)
{
	// put a breakpoint in MissionServer.c around line 1767
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_reportArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, type);
	pktSendStringAligned(pak_out, comment);
	pktSendStringAligned(pak_out, globalName);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_removecomplaint(int arcid, int complainer_id, char *complainer_region_str)
{
	// put a breakpoint in MissionServer.c around line 1856
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_removecomplaint");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, complainer_id);
	pktSendStringAligned(pak_out, complainer_region_str);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_comment(int arcid, int auth_id, const char *comment)
{
	// put a breakpoint in MissionServer.c around line 1887
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_comment");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendStringAligned(pak_out, comment);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_setKeywordsForArc(int accesslevel, int arcid, int auth_id, int keywords)
{
	// put a breakpoint in MissionServer.c around line 1957
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setKeywordsForArc");
	pktSendBitsAuto(pak_out, accesslevel);
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, keywords);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_setCirculationStatus(int accesslevel, int arcid, int auth_id, int circulate)
{
	// put a breakpoint in MissionServer.c around line 2020
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setCirculationStatus");
	pktSendBitsAuto(pak_out, accesslevel);
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, circulate);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_voteForArc(int accesslevel, int arcid, int auth_id, int ivote)
{
	// put a breakpoint in MissionServer.c around line 2105
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_voteForArc");
	pktSendBitsAuto(pak_out, accesslevel);
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, ivote);
	pktSend(&pak_out, &db_comm_link);
}

void missionserver_setGuestBio(int arcid, char *author, char *bio, char *image)
{
	// put a breakpoint in MissionServer.c around line 2215
	Packet *pak_out = pktCreateEx(&db_comm_link, DBCLIENT_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setGuestBio");
	pktSendBitsAuto(pak_out, arcid);
	pktSendStringAligned(pak_out, author);
	pktSendStringAligned(pak_out, bio);
	pktSendStringAligned(pak_out, image);
	pktSend(&pak_out, &db_comm_link);
}

#endif // SERVER

#if CLIENT
#include "clientcomm.h"
#include "entVarUpdate.h"

void missionserver_unpublishArc(int arcid, int auth_id, int accesslevel)
{
	// put a breakpoint in MissionServer.c around line 700
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_unpublishArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, accesslevel);
	END_INPUT_PACKET;
}

void missionserver_updateLevelRange(int arcid, int low, int high)
{
	// put a breakpoint in MissionServer.c around line 752
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_updateLevelRange");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, low);
	pktSendBitsAuto(pak_out, high);
	END_INPUT_PACKET;
}

void missionserver_invalidateArc(int arcid, int unableToPlay, char *error)
{
	// put a breakpoint in MissionServer.c around line 772
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_invalidateArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, unableToPlay);
	pktSendStringAligned(pak_out, error);
	END_INPUT_PACKET;
}

void missionserver_restoreArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 854
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_restoreArc");
	pktSendBitsAuto(pak_out, arcid);
	END_INPUT_PACKET;
}

void missionserver_changeHonors(int arcid, int ihonors)
{
	// put a breakpoint in MissionServer.c around line 909
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_changeHonors");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, ihonors);
	END_INPUT_PACKET;
}

void missionserver_banArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1017
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_banArc");
	pktSendBitsAuto(pak_out, arcid);
	END_INPUT_PACKET;
}

void missionserver_unbannableArc(int arcid)
{
	// put a breakpoint in MissionServer.c around line 1023
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_unbannableArc");
	pktSendBitsAuto(pak_out, arcid);
	END_INPUT_PACKET;
}

void missionserver_grantTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1310
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_grantTickets");
	pktSendBitsAuto(pak_out, tickets);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	END_INPUT_PACKET;
}

void missionserver_grantOverflowTickets(int tickets, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1335
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_grantOverflowTickets");
	pktSendBitsAuto(pak_out, tickets);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	END_INPUT_PACKET;
}

void missionserver_setPaidPublishSlots(int paid_slots, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1370
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setPaidPublishSlots");
	pktSendBitsAuto(pak_out, paid_slots);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	END_INPUT_PACKET;
}

void missionserver_recordArcCompleted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1520
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_recordArcCompleted");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, authid);
	END_INPUT_PACKET;
}

void missionserver_recordArcStarted(int arcid, int authid)
{
	// put a breakpoint in MissionServer.c around line 1621
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_recordArcStarted");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, authid);
	END_INPUT_PACKET;
}

void missionserver_printUserInfo(int auth_id, char *auth_region_str)
{
	// put a breakpoint in MissionServer.c around line 1652
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_printUserInfo");
	pktSendBitsAuto(pak_out, auth_id);
	pktSendStringAligned(pak_out, auth_region_str);
	END_INPUT_PACKET;
}

void missionserver_setBestArcStars(int stars, int db_id, int auth_id)
{
	// put a breakpoint in MissionServer.c around line 1741
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setBestArcStars");
	pktSendBitsAuto(pak_out, stars);
	pktSendBitsAuto(pak_out, db_id);
	pktSendBitsAuto(pak_out, auth_id);
	END_INPUT_PACKET;
}

void missionserver_reportArc(int arcid, int auth_id, int type, const char *comment, const char *globalName)
{
	// put a breakpoint in MissionServer.c around line 1767
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_reportArc");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, type);
	pktSendStringAligned(pak_out, comment);
	pktSendStringAligned(pak_out, globalName);
	END_INPUT_PACKET;
}

void missionserver_comment(int arcid, int auth_id, const char *comment)
{
	// put a breakpoint in MissionServer.c around line 1887
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_comment");
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendStringAligned(pak_out, comment);
	END_INPUT_PACKET;
}

void missionserver_setKeywordsForArc(int accesslevel, int arcid, int auth_id, int keywords)
{
	// put a breakpoint in MissionServer.c around line 1957
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setKeywordsForArc");
	pktSendBitsAuto(pak_out, accesslevel);
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, keywords);
	END_INPUT_PACKET;
}

void missionserver_voteForArc(int accesslevel, int arcid, int auth_id, int ivote)
{
	// put a breakpoint in MissionServer.c around line 2105
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_voteForArc");
	pktSendBitsAuto(pak_out, accesslevel);
	pktSendBitsAuto(pak_out, arcid);
	pktSendBitsAuto(pak_out, auth_id);
	pktSendBitsAuto(pak_out, ivote);
	END_INPUT_PACKET;
}

void missionserver_setGuestBio(int arcid, char *author, char *bio, char *image)
{
	// put a breakpoint in MissionServer.c around line 2215
	START_INPUT_PACKET(pak_out, CLIENTINP_MISSIONSERVER_COMMAND);
	meta_pktSendRoute(pak_out);
	pktSendStringAligned(pak_out, "missionserver_setGuestBio");
	pktSendBitsAuto(pak_out, arcid);
	pktSendStringAligned(pak_out, author);
	pktSendStringAligned(pak_out, bio);
	pktSendStringAligned(pak_out, image);
	END_INPUT_PACKET;
}

#endif // CLIENT
