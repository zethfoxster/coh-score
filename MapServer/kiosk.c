/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "stdtypes.h"
#include "error.h"
#include "earray.h"

#include "entPlayer.h"
#include "grouputil.h"
#include "group.h"
#include "utils.h"
#include "langServerUtil.h"
#include "dbcomm.h"
#include "textparser.h"
#include "svr_base.h"
#include "comm_game.h"
#include "VillainDef.h"
#include "cmdserver.h"
#include "stats_base.h"

#include "kiosk.h"
#include "mathutil.h"
#include "dbnamecache.h"
#include "entity.h"

#include "arenamapserver.h"
#include "badges_server.h"
#include "file.h"
#include "timing.h"

typedef struct Kiosk
{
	char *pchName;
	Vec3 pos;
} Kiosk;

static Kiosk **g_Kiosks = NULL;

/**********************************************************************func*
 * kiosk_Create
 *
 */
static Kiosk *kiosk_Create(void)
{
	return calloc(1, sizeof(Kiosk));
}

/**********************************************************************func*
 * kiosk_Destroy
 *
 */
static void kiosk_Destroy(Kiosk *p)
{
	if(p->pchName)
	{
		free(p->pchName);
	}

	free(p);
}

/**********************************************************************func*
 * kiosk_LocationRecorder
 *
 */
static int kiosk_LocationRecorder(GroupDefTraverser* traverser)
{
	char *pchKiosk;
	Kiosk *pKiosk;

	// We are only interested in nodes with properties.
	if(!traverser->def->properties)
	{
		// We're only interested in subtrees with properties.
		if(!traverser->def->child_properties)
		{
			return 0;
		}

		return 1;
	}

	// Does this thing have a Kiosk?
	pchKiosk = groupDefFindPropertyValue(traverser->def, "Kiosk");
	if(pchKiosk==NULL)
	{
		return 1;
	}

	pKiosk = kiosk_Create();
	copyVec3(traverser->mat[3], pKiosk->pos);
	pKiosk->pchName = strdup(pchKiosk);

	eaPush(&g_Kiosks, pKiosk);

	return 1;
}


/**********************************************************************func*
 * kiosk_Load
 *
 */
void kiosk_Load(void)
{
	GroupDefTraverser traverser = {0};

	loadstart_printf("Loading kiosk locations.. ");

	// If we're doing a reload, clear out old data first.
	if(g_Kiosks)
	{
		eaClearEx(&g_Kiosks, kiosk_Destroy);
		eaSetSize(&g_Kiosks, 0);
	}
	else
	{
		eaCreate(&g_Kiosks);
	}

	groupProcessDefExBegin(&traverser, &group_info, kiosk_LocationRecorder);

	loadend_printf("%i found", eaSize(&g_Kiosks));
}


/**********************************************************************func*
 * DumpLeaders
 *
 */
static void DumpStats(Entity * e, StuffBuff *psb, char *which)
{
	if (!e)
	{
		addStringToStuffBuff(psb, "<span align=center><scale 1.5>%s</scale></span><br>\n", localizedPrintf(e,"KioskWelcome"));
		addStringToStuffBuff(psb, "<span><scale 1.2>\n");
		addStringToStuffBuff(psb, "%s", localizedPrintf(e,"KioskHomePage"));
		addStringToStuffBuff(psb, "<br><br>%s", localizedPrintf(e,"StatsOffline"));
		addStringToStuffBuff(psb, "</scale></span>\n");
	}
	else if (stricmp(which, "character") == 0)
	{
		char time[128];
		addStringToStuffBuff(psb, "<span align=center><scale 1.5>%s: %s</scale></span>\n", "Character", e->name);

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Combat");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Villains Defeated:", getCommaSeparatedInt(badge_StatGet(e, "kills")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Players Defeated:", getCommaSeparatedInt(badge_StatGet(e, "kills.arena") + badge_StatGet(e, "kills.pvp")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Damage Inflicted:", getCommaSeparatedInt(badge_StatGet(e, "damage.given")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Damage Taken:", getCommaSeparatedInt(badge_StatGet(e, "damage.taken")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Healing Given:", getCommaSeparatedInt(badge_StatGet(e, "healing.given")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Time Held:", timerMakeOffsetStringFromSeconds(time, badge_StatGet(e, "time.held")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Time Immobilized:", timerMakeOffsetStringFromSeconds(time, badge_StatGet(e, "time.immobilized")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Time Slept:", timerMakeOffsetStringFromSeconds(time, badge_StatGet(e, "time.sleep")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Time Stunned:", timerMakeOffsetStringFromSeconds(time, badge_StatGet(e, "time.stunned")));
		addStringToStuffBuff(psb, "</table>");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Missions");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Tasks Completed:", getCommaSeparatedInt(badge_StatGet(e, "task.complete")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Tasks Failed:", getCommaSeparatedInt(badge_StatGet(e, "task.failed")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Flashback Arcs Completed:", getCommaSeparatedInt(badge_StatGet(e, "FlashbacksComplete")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Reward Merits Earned:", getCommaSeparatedInt(badge_StatGet(e, "drops.merit_rewards")));
		addStringToStuffBuff(psb, "</table>");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Other");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Total XP Earned:", getCommaSeparatedInt(badge_StatGet(e, "xp")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "XP Debt Acquired:", getCommaSeparatedInt(badge_StatGet(e, "xpdebt")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Inventions Created:", getCommaSeparatedInt(badge_StatGet(e, "invention.Created")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Invention Salvage Received:", getCommaSeparatedInt(badge_StatGet(e, "drops.invention_salvage")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Visits to the top of Atlas' Statue:", getCommaSeparatedInt(badge_StatGet(e, "visits.AtlasPark5")));
		addStringToStuffBuff(psb, "</table>");
	}
	else if (stricmp(which, "architect") == 0)
	{
		addStringToStuffBuff(psb, "<span align=center><scale 1.5>%s</scale></span>\n", "Architect Entertainment");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Story Arcs Completed");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Dev Choice:", getCommaSeparatedInt(badge_StatGet(e, "Architect.PlayDevChoice")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Hall of Fame:", getCommaSeparatedInt(badge_StatGet(e, "Architect.PlayHallofFame")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Guest Author:", getCommaSeparatedInt(badge_StatGet(e, "Architect.PlayFeatured")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Hero Morality:", getCommaSeparatedInt(badge_StatGet(e, "Architect.HeroMissions")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Villain Morality:", getCommaSeparatedInt(badge_StatGet(e, "Architect.VillainMissions")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Rogue Morality:", getCommaSeparatedInt(badge_StatGet(e, "Architect.RogueMissions")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Vigilante Morality:", getCommaSeparatedInt(badge_StatGet(e, "Architect.VigilanteMissions")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Test Mode:", getCommaSeparatedInt(badge_StatGet(e, "Architect.TestPlay")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Total:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Play") + badge_StatGet(e, "Architect.TestPlay")));
		addStringToStuffBuff(psb, "</table>");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Mission Objectives");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Collections Obtained:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Click") + badge_StatGet(e, "Architect.ClickTest")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Objects Destroyed:",getCommaSeparatedInt(badge_StatGet(e, "Architect.Destroy") + badge_StatGet(e, "Architect.TestDestroy")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Hostages Rescued:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Rescue") + badge_StatGet(e, "Architect.TestRescue")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Non-Required Objectives:", getCommaSeparatedInt(badge_StatGet(e, "Architect.NonRequiredObjective")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Custom Enemies Defeated:", getCommaSeparatedInt(badge_StatGet(e, "Architect.CustomKills")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Total Enemies Defeated:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Kill") + badge_StatGet(e, "Architect.TestKill")));
		addStringToStuffBuff(psb, "</table>");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Rewards");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Tickets Earned:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Tickets")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Test Mode Tickets:", getCommaSeparatedInt(badge_StatGet(e, "Architect.TestTickets")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Overflow Tickets Claimed:", getCommaSeparatedInt(badge_StatGet(e, "Architect.OverflowTickets")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Pending Overflow Tickets:", getCommaSeparatedInt(badge_StatGet(e, "Architect.PendingOverflowTickets")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Inspirations Received:", getCommaSeparatedInt(badge_StatGet(e, "Architect.Inspirations")));
		addStringToStuffBuff(psb, "</table>");
	}
	else if (stricmp(which, "market") == 0)
	{
		addStringToStuffBuff(psb, "<span align=center><scale 1.5>%s</scale></span>\n", "Market");

		addStringToStuffBuff(psb, "<br><span align=center><scale 1.5>%s</scale></span>\n", "Items Sold");
		addStringToStuffBuff(psb, "<scale 1.0><table>");
		addStringToStuffBuff(psb, "<tr><td width=50 align=right><bsp>%s</td><td>%s</td></tr>", "Enhancements:", getCommaSeparatedInt(badge_StatGet(e, "auction.sold.boost")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Inspirations:", getCommaSeparatedInt(badge_StatGet(e, "auction.sold.inspiration")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Recipes:", getCommaSeparatedInt(badge_StatGet(e, "auction.sold.recipe")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Salvage:", getCommaSeparatedInt(badge_StatGet(e, "auction.sold.salvage")));
		addStringToStuffBuff(psb, "<tr><td align=right><bsp>%s</td><td>%s</td></tr>", "Total:", getCommaSeparatedInt(badge_StatGet(e, "auction.sold")));
		addStringToStuffBuff(psb, "</table>");
	}
	else
	{
		addStringToStuffBuff(psb, "<span align=center><scale 1.5>%s</scale></span><br>\n", localizedPrintf(e,"KioskWelcome"));
		addStringToStuffBuff(psb, "<span><scale 1.2>\n");
		addStringToStuffBuff(psb, "%s", localizedPrintf(e,"KioskHomePage"));
		addStringToStuffBuff(psb, "</scale></span>\n");
	}
}


/**********************************************************************func*
 * kiosk_FindClosest
 *
 */
Kiosk *kiosk_FindClosest(const Vec3 vec)
{
	float fDist = SQR(20);
	Kiosk *pKiosk = NULL;
	int i;

	for(i=eaSize(&g_Kiosks)-1; i>=0; i--)
	{
		float fNewDist = distance3Squared(vec, g_Kiosks[i]->pos);
		if(fNewDist < fDist)
		{
			pKiosk = g_Kiosks[i];
			fDist = fNewDist;
		}
	}

	return pKiosk;
}

/**********************************************************************func*
 * kiosk_Tick
 *
 */
void kiosk_Tick(Entity *e)
{
	if(e->pl->at_kiosk)
	{
		kiosk_Check(e);
	}
}

/**********************************************************************func*
 * kiosk_Check
 *
 */
bool kiosk_Check(Entity *e)
{
	if(kiosk_FindClosest(ENTPOS(e))==NULL)
	{
		START_PACKET(pak, e, SERVER_BROWSER_CLOSE);
		END_PACKET
		e->pl->at_kiosk = 0;
		return false;
	}

	return true;
}

/**********************************************************************func*
 * kiosk_Navigate
 *
 */
void kiosk_Navigate(Entity *e, char *cmd, char *cmd2)
{
	StuffBuff sb;

	if(!kiosk_Check(e))
	{
		return;
	}

	e->pl->at_kiosk = 1;

	initStuffBuff(&sb, 128);

	addStringToStuffBuff(&sb, "<font face=computer outline=0 color=#32ff5d><b>\n");
	addStringToStuffBuff(&sb, "<span align=center>\n");
	addStringToStuffBuff(&sb, "<scale 1.8>%s</scale><br><br>\n", localizedPrintf(e,"KioskCityInfo"));
	addStringToStuffBuff(&sb, "</span>\n");

	addStringToStuffBuff(&sb, "<table><tr>\n");

	addStringToStuffBuff(&sb, "<td>\n");
		addStringToStuffBuff(&sb, "<font face=computer outline=0 color=#22ff4d><b>\n"); // #a9e3f7
		addStringToStuffBuff(&sb, "<linkbg #88888844><linkhoverbg #578836><linkhover #a7ff6c><link #52ff7d><b>\n");
		addStringToStuffBuff(&sb, "<table>\n");
		addStringToStuffBuff(&sb, "<a href='cmd:kiosk home home'><tr%s><td align=center valign=center>%s</td></tr></a>\n", stricmp(cmd,"home")==0?" selected=1":"", localizedPrintf(e,"KioskHome"));
		addStringToStuffBuff(&sb, "<tr><td><br></td></tr>\n");
		addStringToStuffBuff(&sb, "<tr><td align=left valign=center>%s</td></tr>", localizedPrintf(e, "TopicString") );
			addStringToStuffBuff(&sb, "<a href='cmd:kiosk character %s'><tr%s><td align=right valign=center>%s</td></tr></a>\n", cmd2, stricmp(cmd,"character")==0?" selected=1":"", "Character");
			addStringToStuffBuff(&sb, "<a href='cmd:kiosk architect %s'><tr%s><td align=right valign=center>%s</td></tr></a>\n", cmd2, stricmp(cmd,"architect")==0?" selected=1":"", "Architect");
			addStringToStuffBuff(&sb, "<a href='cmd:kiosk market %s'><tr%s><td align=right valign=center>%s</td></tr></a>\n", cmd2, stricmp(cmd,"market")==0?" selected=1":"", "Market");
		addStringToStuffBuff(&sb, "</table>\n");
		addStringToStuffBuff(&sb, "</linkhover>\n");
		addStringToStuffBuff(&sb, "</linkhoverbg>\n");
		addStringToStuffBuff(&sb, "</linkbg>\n");
		addStringToStuffBuff(&sb, "</font>\n");
	addStringToStuffBuff(&sb, "</td>\n");

	addStringToStuffBuff(&sb, "<td>\n");
	addStringToStuffBuff(&sb, "<img src=white.tga height=275 width=1 color=#32ff5d><br>\n");
	addStringToStuffBuff(&sb, "</td>\n");

	addStringToStuffBuff(&sb, "<td width=90>\n");
	DumpStats(e, &sb, cmd);
	addStringToStuffBuff(&sb, "</td>\n");

	addStringToStuffBuff(&sb, "</tr></table>\n");
	addStringToStuffBuff(&sb, "</b></font>\n");

	START_PACKET(pak, e, SERVER_BROWSER_TEXT);
	pktSendString(pak, sb.buff);
	END_PACKET

	freeStuffBuff(&sb);
}

/* End of File */
