
//////////////////////////////////////////////////////////////////////////
//
// MISSION SCRIPT
// Encounter Script
//
//
//
//////////////////////////////////////////////////////////////////////////
//
//	Praetorian Malaise Event script
//	Malaise has a few oddities that make him different from most other scripts
//	His main schtick will teleport the 2 highest aggro players into 2 seperate rooms
//	Designers will be able to specify:
//		1.) at what health does he teleport
//		2.) where does he teleport each player

#include "scriptutil.h"
#include "scriptengine.h"
#include "entity.h"
#include "mission.h"
#include "Scripthook\ScriptHookInternal.h"
#include "entaiprivate.h"
#include "character_tick.h"
#include "Quat.h"

#define MAX_MID_POINTS 5
#define MAX_POINTS (MAX_MID_POINTS+2)
#define END_POINT (MAX_MID_POINTS+1)
#define MAX_TELEPORT_LOCATIONS 3
static struct Malaise_teleport_names {
	char *port_location;
	char *port_spawndef;
	char *port_actor_name;
	char *port_entID;
	char *port_returnLoc;
} malaise_teleport_names[MAX_TELEPORT_LOCATIONS+1] = 
{
	{	"BOSS_PORT_LOC_1",	"BOSS_SPAWNDEF_1",		"PORT_ACTOR_NAME_1",		"ENT_ID_1",		"ENT_RETURN_POS_1"		},
	{	"BOSS_PORT_LOC_2",	"BOSS_SPAWNDEF_2",		"PORT_ACTOR_NAME_2",		"ENT_ID_2",		"ENT_RETURN_POS_2"		},
	{	"BOSS_PORT_LOC_3",	"BOSS_SPAWNDEF_3",		"PORT_ACTOR_NAME_3",		"ENT_ID_3",		"ENT_RETURN_POS_3"		},
	{	"BOSS_PORT_BASE",	"BOSS_SPAWNDEF_BASE",	"PORT_ACTOR_NAME_BASE",		"ENT_ID_BASE",	"ENT_RETURN_POS_BASE"	},
};
static struct Malaise_trigger_names {
	char *health;
	char *dialog;
	char *do_ai;
	char *timer;
	char *num_taken;
	char *timer_complete_ai;
	char *done;
} malaise_trigger_names[MAX_POINTS] = {
	{"",					"BOSS_1STBLOOD_DIALOG",	"",				"",						"",						"",							"DONE_1STBLOOD"},
	{"BOSS_1ST_HEALTH",		"BOSS_1ST_DIALOG",		"1ST_DO_AI",	"BOSS_1ST_TIMER",		"BOSS_1ST_TAKE",		"1ST_TIMER_COMPLETE_AI",	"DONE_1ST"},
	{"BOSS_2ND_HEALTH",		"BOSS_2ND_DIALOG",		"2ND_DO_AI",	"BOSS_2ND_TIMER",		"BOSS_2ND_TAKE",		"2ND_TIMER_COMPLETE_AI",	"DONE_2ND"},
	{"BOSS_3RD_HEALTH",		"BOSS_3RD_DIALOG",		"3RD_DO_AI",	"BOSS_3RD_TIMER",		"BOSS_3RD_TAKE",		"3RD_TIMER_COMPLETE_AI",	"DONE_3RD"},
	{"BOSS_4TH_HEALTH",		"BOSS_4TH_DIALOG",		"4TH_DO_AI",	"BOSS_4TH_TIMER",		"BOSS_4TH_TAKE",		"4TH_TIMER_COMPLETE_AI",	"DONE_4TH"},
	{"BOSS_5TH_HEALTH",		"BOSS_5TH_DIALOG",		"5TH_DO_AI",	"BOSS_5TH_TIMER",		"BOSS_5TH_TAKE",		"5TH_TIMER_COMPLETE_AI",	"DONE_5TH"},
	{"",					"BOSS_DEAD_DIALOG",		"",				"BOSS_DEAD_Objective",	"",						"",							"DONE_DEAD"}
};

static char TIMER[] = "TIMER";
static char DO_AI[] = "DO_AI";
static char TIMER_COMPLETE_AI[] = "TIMER_COMPLETE_AI";

static int find_next_health(int mid_point)
{
	for (;;)
	{
		mid_point++;
		if (mid_point == END_POINT) 
			break;
		if (VarGetFraction(malaise_trigger_names[mid_point].health) > 0.0)
		{
			return mid_point;
		}
	}
	return END_POINT;
}

static int find_last_point(void)
{
	int j;

	for (j=MAX_MID_POINTS;j>0;j--)
	{
		if ( !VarIsEmpty(malaise_trigger_names[j].health) || 
			!VarIsEmpty(malaise_trigger_names[j].dialog) ||
			!VarIsEmpty(malaise_trigger_names[j].timer) ||
			!VarIsEmpty(malaise_trigger_names[j].num_taken) ||
			!VarIsEmpty(malaise_trigger_names[j].do_ai) ||
			!VarIsEmpty(malaise_trigger_names[j].timer_complete_ai) )
		{
			return j;
		}
	}
	return 0;
}

static void spawnMalaiseClones(LOCATION loc, int index, const Vec3 newPos, const Mat3 mat, FRACTION health)
{
	ENCOUNTERGROUP group;
	group = FindEncounterGroupByRadius( 
		loc,
		0, 
		300, //Within 300 feet of this encounter
		"Around",
		1, 
		0 );

	if( group && !StringEqual( group, LOCATION_NONE ) )
	{
		LOCATION loc;
		const char *spawnDefFileName = StringAdd("BOSS_SPAWNDEF_", NumberToString(index+1) );
		const char *spawnDefFile = VarGet(spawnDefFileName);
		loc = Spawn( 1, "MalaiseClones", spawnDefFile, group, "Around", 0, 0 );
		if( loc && !StringEqual( loc, LOCATION_NONE ) )
		{
			Entity *boss;
			SetEncounterActive( group );
			boss = EntTeamInternal(group, 0, NULL);
			if (boss)
			{
				VarSet(malaise_teleport_names[index].port_actor_name, EntityNameFromEnt(boss));
				entUpdatePosInstantaneous(boss, newPos);
				entSetMat3(boss, mat);
				health = CLAMP(health, 0.0f, 1.0f );
				if (boss->pchar)
				{
					boss->pchar->attrCur.fHitPoints = health * boss->pchar->attrMax.fHitPoints;
					character_HandleDeathAndResurrection(boss->pchar, NULL);
				}
				DetachSpawn( group );
			}
		}
	}
}
//	teleport targets
//	The boss will select x targets to teleport to identical copies of the base room,
//	where x is num_taken as sepcified by the script.
//	In order for this to work, the rooms are stacked on top of each other. A marker is placed in each room
//	in the same relative location, by using these markers, the script can figure out a relative y offset to
//	apply to the player in order to teleport him to the room in the same relative location.
static void teleportTargets(int n, ENTITY boss)
{
	Entity *boss_ent = EntTeamInternal(boss, 0, NULL);
	if (boss_ent)
	{
		const char *base_location = VarGet(malaise_teleport_names[MAX_TELEPORT_LOCATIONS].port_location);
		if (base_location[0])
		{
			Vec3 basePos;
			if (GetPointFromLocation(StringAdd("marker:", base_location), basePos))
			{
				int last_db_id = 0;
				AIVars *ai = ENTAI(boss_ent);
				int num_taken = VarGetNumber(malaise_trigger_names[n].num_taken);
				int i;
				for (i = 0; i < MIN(num_taken, MAX_TELEPORT_LOCATIONS); ++i)
				{	
					Entity *target = aiSetAttackTargetToAggroPlayer(boss_ent, ai, i+1);
					if( target )
					{
						if (target->db_id == last_db_id)
						{
							//	Kind of a hack here: 
							//	the setAttackTarget can sometimes return the same player
							//	if there are less players than the aggro index
							//	it returns the lowest aggro (highest index) player
							//	we can detect when we're out of players when the last dbid is equal to the previous one
							break;
						}
						else
						{
							Vec3 markerPos;
							const char *marker_location;
							marker_location = VarGet(malaise_teleport_names[i].port_location);
							if (marker_location[0])
							{
								if (GetPointFromLocation(StringAdd("marker:", marker_location), markerPos) )
								{
									Vec3 markerOffset;
									Vec3 targetPos, bossPos;
									Mat4 targetMat, bossMat;
									copyVec3(ENTPOS(target), targetPos);
									copyVec3(ENTPOS(boss_ent), bossPos);
									subVec3(markerPos, basePos, markerOffset);
									addVec3(targetPos, markerOffset, targetPos);
									addVec3(bossPos, markerOffset, bossPos);
									VarSet(malaise_teleport_names[i].port_returnLoc, EntityPos(StringAdd("player:", NumberToString(target->db_id))) );
									VarSetNumber(malaise_teleport_names[i].port_entID, target->db_id);
									quatToMat(target->net_qrot, targetMat);
									entSetMat3(target, targetMat);
									entUpdatePosInstantaneous( target, targetPos );
									quatToMat(boss_ent->net_qrot, bossMat);
									spawnMalaiseClones(StringAdd("marker:", marker_location), i, bossPos, bossMat, GetHealth(boss));
									last_db_id = target->db_id;
									//	TODO:	spawn me at the new position as well
								}
							}
						}
					}
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// Helper function.
//

static void do_mid_point(int n, ENTITY boss)
{
	NUMBER timer;
	const char *do_ai;
	const char *timer_complete_ai;
	//Say won't do anything if passed ""
	Say( boss, VarGet(malaise_trigger_names[n].dialog), 2 );

	// complete objective if specified
	timer = VarGetNumber(malaise_trigger_names[n].timer);
	if (timer)
	{
		//	set timer til objective done
		VarSetNumber(TIMER, CurrentTime()+timer);
	}

	//	 teleport the players to the necessary locations
	teleportTargets(n, boss);

	//Do something, if told to.
	do_ai = VarGet(malaise_trigger_names[n].do_ai);
	if (do_ai[0])
	{
		AddAIBehavior(boss, do_ai);
		VarSet(DO_AI, do_ai);
	}

	timer_complete_ai = VarGet(malaise_trigger_names[n].timer_complete_ai);
	if (timer_complete_ai[0])
	{
		VarSet(TIMER_COMPLETE_AI, timer_complete_ai);
	}

}
static FRACTION killMalaiseClones(int index)
{
	ENTITY boss = VarGet(malaise_teleport_names[index].port_actor_name);
	FRACTION returnHealth = GetHealth(boss);
	SetHealth(boss, 0.0f, 0);
	VarSet(malaise_teleport_names[index].port_actor_name, "");
	return returnHealth;
}
static void returnTeleportTargets(ENTITY boss)
{
	int i;
	const char* base_location;
	base_location = VarGet(malaise_teleport_names[MAX_TELEPORT_LOCATIONS].port_location);
	if (base_location[0])
	{
		Vec3 base_pos;
		if (GetPointFromLocation(StringAdd("marker:", base_location), base_pos))
		{
			FRACTION averageHealth = 0;
			int count = 0;
			for (i = 0; i < MAX_TELEPORT_LOCATIONS; ++i)
			{
				const char *ent_db_id = VarGet(malaise_teleport_names[i].port_entID);
				if (ent_db_id[0])
				{
					Entity *player = EntTeamInternal(StringAdd("player:", ent_db_id), 0, NULL);
					if (player)
					{
						//	Method 1:
						//	Teleport them and maintain their relative position
						//	Vec3 marker_pos;
// 							if (GetPointFromLocation(StringAdd("marker:", VarGet(malaise_teleport_names[i].port_location)), marker_pos))
// 							{
// 								Vec3 marker_offset;
// 								Vec3 player_pos;
// 								copyVec3(ENTPOS(player), player_pos);
// 								subVec3(base_pos, marker_pos, marker_offset);
// 								addVec3(player_pos, marker_offset, player_pos);
// 								entUpdatePosInstantaneous(player, player_pos);
// 							}
						//	Method 2:
						//	Teleport them back to their original position
						const char *returnLocation;
						Vec3 returnPos;
						returnLocation = VarGet(malaise_teleport_names[i].port_returnLoc);
						if (returnLocation[0] && GetPointFromLocation(returnLocation, returnPos))
						{
							entUpdatePosInstantaneous(player, returnPos);
							VarSet(malaise_teleport_names[i].port_returnLoc, "");
						}
						else
						{
							assert(0);	//	WTF, we already established this marker has to exist since they were teleported there o.O =/
						}
					}
					else
					{
						//	CRAAAPP! the player dropped while he was teleported away.. what now?
						//	change his position so that when he returns, he isn't trapped in el roomo de boxo
					}
					averageHealth += killMalaiseClones(i);
					count++;
				}
				VarSet(malaise_teleport_names[i].port_entID, "");
			}
			SetHealth(boss, averageHealth/count, 0);
		}
	}
}

void MalaiseEvent(void)
{  

	INITIAL_STATE 
	{
		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY
		{
			ENTITY boss;
			int j;
			FRACTION health_amount;
			FRACTION next_health_amount;
			FRACTION delta;
			int mid_point;
			int next_mid_point;
			int fill_point;
			int num_mid_points;
			int n;

			// First, count up how many midpoints there are.
			num_mid_points = find_last_point();

			// set mid_points defaults for those not specified.
			// pick an even spread of values between each two specified values.
			health_amount = 1.0;	//Start at full health
			mid_point = 0;

			while(mid_point < END_POINT)
			{
				next_mid_point = find_next_health(mid_point);
				if (next_mid_point == END_POINT)	//no more defined
				{
					fill_point = num_mid_points+1; //fill to end.
					next_health_amount = 0.0;
				} else {
					fill_point = next_mid_point;
					next_health_amount = VarGetFraction(malaise_trigger_names[fill_point].health);
				}

				// Set defaults between two defined values
				n = (fill_point - mid_point);	//n = number of points between those + 1
				if (n>0)
					delta = (health_amount-next_health_amount)/n;
				for(j=mid_point+1; j<fill_point; j++)
				{
					health_amount -= delta;
					VarSetFraction(malaise_trigger_names[j].health, health_amount);
				}

				mid_point = next_mid_point;
				health_amount = next_health_amount;
			}
			//-- 

			boss = ActorFromEncounterActorName(MyEncounter(), VarGet( "Actor" ) );

			if( !boss || StringEqual(boss, TEAM_NONE))
				EndScript();

			SayOnKillHero(boss, VarGet("BOSS_KILLEDHERO_DIALOG") );

			VarSet( "HealthObjectiveBoss", boss );
			VarSetFraction( "LastHealth", GetHealth( boss ) );

		}
		END_ENTRY

		{
			ENTITY boss;
			FRACTION health;
			FRACTION lastHealth;
			FRACTION tp;
			int j;
			int timerEnd;
			const char *do_ai;

			boss = VarGet( "HealthObjectiveBoss" );

			timerEnd = VarGetNumber(TIMER);
			if (timerEnd)
			{
				if (CurrentTime() > timerEnd)
				{
					const char *timer_complete_ai;
					//Once complete, clear flags.
					VarSetNumber(TIMER, 0);

					do_ai = VarGet(DO_AI);
					if (do_ai[0])
					{
						RemoveAllBehaviors(boss);
					}

					//if port_to was set, then go back there
					returnTeleportTargets(boss);

					timer_complete_ai = VarGet(TIMER_COMPLETE_AI);
					if (timer_complete_ai[0])
					{
						AddAIBehavior(boss, timer_complete_ai);
					}
				}
			}
			else
			{
				health = GetHealth( boss );
				lastHealth = VarGetFraction( "LastHealth" );

				if (( lastHealth >= 1.00 && 1.00 > health ) && !VarGetNumber( "DONE_1STBLOOD" ) )
				{
					do_mid_point(0, boss);
					VarSetNumber( "DONE_1STBLOOD", 1 ); 
				}

				for (j=1; (health > 0.0f) && (j<=MAX_MID_POINTS); j++)
				{
					tp = VarGetFraction(malaise_trigger_names[j].health);
					if ((tp > 0.0) && (health < tp) && !VarGetNumber( malaise_trigger_names[j].done ) && VarGetNumber(malaise_trigger_names[j-1].done) )
					{
						do_mid_point(j, boss);
						VarSetNumber( malaise_trigger_names[j].done, 1 );
						break;			//	only do 1 of these per tick
					}
				}

				if( lastHealth > 0.00 && 0.00 >= health )//&& !VarGetNumber( "DoneDead" ) )
				{
					Say( boss, VarGet( "BOSS_DEAD_DIALOG" ), 2 ); 
					VarSetNumber( "DoneDead", 1 ); 
				}

				VarSetFraction( "LastHealth", health );
			}
		}
	}
	END_STATES 
}

void MalaiseEventInit()
{
	int j;

	SetScriptName( "MalaiseEvent" );     
	SetScriptFunc( MalaiseEvent );
	SetScriptType( SCRIPT_ENCOUNTER );		

	SetScriptVar( "Actor",								"",		0 );

	SetScriptVar( "BOSS_1STBLOOD_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL ); 

	for (j=1; j<=MAX_MID_POINTS; j++)
	{
		SetScriptVar( malaise_trigger_names[j].health,		"",		SP_OPTIONAL ); 
		SetScriptVar( malaise_trigger_names[j].dialog,		"",		SP_MULTIVALUE | SP_OPTIONAL ); 
		SetScriptVar( malaise_trigger_names[j].do_ai,		"",		SP_OPTIONAL ); 
		SetScriptVar( malaise_trigger_names[j].timer,	"",		SP_OPTIONAL ); 
		SetScriptVar( malaise_trigger_names[j].num_taken,	"",		SP_OPTIONAL ); 
		SetScriptVar( malaise_trigger_names[j].timer_complete_ai,	"",		SP_OPTIONAL ); 
	}
	for (j=0; j<= MAX_TELEPORT_LOCATIONS; j++)							//	we also include the base location in here
	{
		SetScriptVar( malaise_teleport_names[j].port_location, "",	SP_OPTIONAL );
		SetScriptVar( malaise_teleport_names[j].port_actor_name, "", SP_OPTIONAL );
		SetScriptVar( malaise_teleport_names[j].port_entID, "",	SP_OPTIONAL );
		SetScriptVar( malaise_teleport_names[j].port_spawndef, "",	SP_OPTIONAL );
		SetScriptVar( malaise_teleport_names[j].port_returnLoc, "",	SP_OPTIONAL );
	}

	SetScriptVar( "BOSS_DEAD_DIALOG",					"",		SP_MULTIVALUE | SP_OPTIONAL ); 

	SetScriptVar( "BOSS_KILLEDHERO_DIALOG",				"",		SP_MULTIVALUE | SP_OPTIONAL );
}
