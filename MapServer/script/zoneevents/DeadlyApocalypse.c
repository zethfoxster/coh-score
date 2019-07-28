// ZONE EVENT SCRIPT

// This script runs the Deadly Apocalypse Halloween 2009 Event.
//

#include "scriptutil.h"
#include "scriptengine.h"
#include "file.h"
#include "gridcoll.h"
#include "svr_chat.h"
#include "timing.h"
#include <assert.h>

// Relative probabilities of the four ranks when spawning critters.
#define MINION_PROBABILITY			15
#define LT_PROBABILITY				15
#define BOSS_PROBABILITY			35
#define EB_PROBABILITY				35

// How many players are each of the defender ranks worth
#define DEFENDER_VALUE_MINION		0.3f
#define DEFENDER_VALUE_LT			0.6f
#define DEFENDER_VALUE_BOSS			1.0f
#define DEFENDER_VALUE_EB			2.5f

// Tunable value.  Radius around sigil where ambushes spawn.
#define FINAL_AMBUSH_RADIUS			30.0f

// Tunable value.  Radius around sigil where returning kited mnobs will go to.
#define FINAL_RETURN_RADIUS			30.0f 

// Tunable value.  Radius around sigil where we consider players as part of the "attacking" team
#define PLAYER_INCLUDE_RADIUS		100.0f

// Tunable value.  Radius around sigil where we count critters to determine the sigil buff power
#define CRITTER_COUNT_RADIUS		50.0f

// Tunable value.  Radius around sigil within which "Sigil vulnerable" and "Find the boss" messages are broadcast.
#define PLAYER_MESSAGE_RADIUS		250.0f

// Tunable value.  Minimum distance a player has to be from a sigil before the UI appears
#define UI_APPEAR_RADIUS			200.0f

// Tunable value.  Maximum distance a player has to be from a sigil before the UI disappears
#define UI_DISAPPEAR_RADIUS			250.0f

#define	STATE_CHANGE_DELAY			1.0f

#define SigilLocationCount			10
#define SigilLocationTemplate		"DA_Sigil_%02d"
#define AmbushLocationCount			4
#define AmbushLocationTemplate		"DA_Ambush_%02d_%02d"
#define BossLocationCount			10
#define BossLocationTemplate		"DA_Boss_%02d"

#define	TEMPLATE_RESULT_SIZE		64

// Maximum number of players we'll consider when counting them in the locality of a sigil
#define MAXENTS 50

#define CLEANUP_PL					"DoNotGoToSleep, DoNothing(Untargetable,AnimList(Dismiss),Timer(2)), RunIntoDoor"
#define	SIGIL_INVULNERABLE_PL		"DoNothing(Untargetable(1),Unselectable(1),DoNotFaceTarget(1),DoAttackAI(0))"
#define	SIGIL_VULNERABLE_PL			"DoNothing(Untargetable(0),Unselectable(0),DoNotFaceTarget(1),DoAttackAI(0))"
#define	DEFENDER_SPAWN_PL			"DoNothing(Untargetable(1),Unselectable(1),Timer(2)), DoNothing(Untargetable(0),Unselectable(0),Timer(1)), Combat"

// maximum number of waves allowed
#define MAX_WAVE	8

// Yes, these are ints masquerading as #defines.  They are named this way since semantically they *ARE* #defines, it's just that
// the numeric values are in the scriptdef, and these are read from there at script startup time.
static int MAX_DEFENDERS;
static int MAX_DEFENDER_WANDER;
static int DEFENDER_KILLS_NEEDED;

// HACK HACK HACK
// This is a very nasty trick that will let me push a feature live (maybe) after the fact.
// This token will initially not be live.  If we decide after playtest on the PTS that this can be
// turned on, then simply using one of the following slash commands will do so.
// /maprwtoken EnableSpawnsAtAspect 1 .. 4
// where the number 1 thru 4 dials how agressively they spawn.
static int ENABLE_SPAWNS_AT_ASPECT;
// Likewise this one turns on some code that throttles how fast they can kill him.  This simply says the minimum number of minutes the fight will last
static int THROTTLE_ASPECT_DAMAGE;

//static int sigilMapMarkers[MAX_WAVE];
static int sigilMapMarkers[10];		// TEMPORARY, I HOPE HOPE HOPE

// values that say what grades of critters to spawn
#define SPAWN_LT_AND_MINION		0
#define SPAWN_BOSS				1
#define SPAWN_EB				2

// Useful for debug output
static void chatSendDebugPrintf(char *format, ...)
{
	va_list ap;
	char buf[16384];

	if (isDevelopmentMode())
	{
		va_start(ap, format);
		vsnprintf(buf, 16383, format, ap);
		buf[16383] = 0;
		va_end(ap);
		chatSendDebug(buf);
	}
}

/////////////////////////////////////////////////////////////////////////
// Raise a number to a power, except don't let it exceed the cap
// This is used to set the multiplicative kill target for defenders
NUMBER cappedIntegerPower(NUMBER base, NUMBER power, NUMBER cap)
{
	int i;
	__int64 result;

	result = 1;
	for (i = 0; i < power; i++)
	{
		result = result * base;
		if (result >= (__int64) cap)
		{
			result = cap;
			break;
		}
	}
	// Yes - this appears that it might lose precision, casting a 64 bit int to 32.
	// But since result can not exceed cap, and cap is a 32 bit int, that means this
	// will work OK
	return (NUMBER) result;
}

/////////////////////////////////////////////////////////////////////////
// Find a safe place to plonk down a spawn.  Spread out from the given center by the variation distance,
// but then use a probe from above to find the correct y value.  The probe solves the problem of a target
// centerpoint that is is a bowl with a radius smaller than the variation distance.  If we simply spread
// out on a plane in that case, the target location can be "outside the geometry" which causes all sorts
// of problems, including crashing mapservers.
STRING GetSafeTargetPos(LOCATION centerLoc, FRACTION variation)
{
	int i;
	Vec3 centerPos;
	Vec3 floorPos;
	Vec3 highPos;
	CollInfo coll;
	char finalLocStr[256];

	if (!GetPointFromLocation(centerLoc, centerPos))
	{
		if (isDevelopmentMode())
		{
			assert(0 && "Bad location handed to GetSafeTargetPos(...)");
		}
		return LOCATION_NONE;
	}
	for (i = 0; i < 7; i++)	
	{
		highPos[0] = floorPos[0] = centerPos[0] + RandomFraction(-variation, variation);
		floorPos[1] = centerPos[1] -20.0f ;
		highPos[1] = centerPos[1] + 20.0f;
		highPos[2] = floorPos[2] = centerPos[2] + RandomFraction(-variation, variation);

		if (collGrid(NULL, highPos, floorPos, &coll, 0.0f, COLL_NOTSELECTABLE|COLL_DISTFROMSTART|COLL_BOTHSIDES))
		{
			floorPos[1] = coll.mat[3][1] + 0.5f;
			snprintf(finalLocStr, sizeof(finalLocStr), "coord:%f,%f,%f", floorPos[0], floorPos[1], floorPos[2]);
			finalLocStr[sizeof(finalLocStr) - 1] = 0;
			return StringDupe(finalLocStr);
		}
		if (i != 0 && variation >= 10.0f)
		{
			variation -= 5.0f;
		}
	}
	return LOCATION_NONE;
}

/////////////////////////////////////////////////////////////////////////
// Counting Ents
// This returns a true count of players, but returns a weighted count of hostiles.  The intention
// being to represent that an EB is more dangerous than a minion.  This counts players and defenders
// for the sigil whose number is given
static void DeadlyApocalypseCountEnts(NUMBER *players, NUMBER *defenders, NUMBER *powerDefenders, int wave)
{
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER defenderCount;
	FRACTION defenderWeight;
	STRING defenderClass;
	STRING searchLocation;
	STRING critterTeam;
	char waveAsString[4];


	if (wave != -1)
	{
		waveAsString[0] = wave + '0';
		waveAsString[1] = 0;
		searchLocation = VarGet(StringAdd("SigilLocations_", waveAsString));
		critterTeam = StringAdd("WaveTeam.", waveAsString);
	}
	else
	{
		searchLocation = VarGet("BossLocation");
		critterTeam = "EventTeam";
	}

	// Count all defenders
	if (defenders != NULL)
	{
		defenderWeight = 0.0f;
		defenderCount = GetAllEntities( entlist, 
									0,													// villain group
									0,													// villain def - Not yet implemented	
									0,													// villain tag - Not yet implemented
									0,													// ent type - Not yet implemented
									critterTeam,										// script team
									PLAYER_INCLUDE_RADIUS,								// radius to check 
									searchLocation,										// location to check
									0,													// area
									1,													// isAlive
									0,													// isDead
									MAXENTS,											// max ents
									0													// only once
									);

		for (i = 0; i < defenderCount; i++)
		{
			defenderClass = GetVillainDefClassName(entlist[i]);
			if (StringEqual(defenderClass, "Class_Boss_Elite"))
			{
				defenderWeight += DEFENDER_VALUE_EB;
			}
			if (StringEqual(defenderClass, "Class_Boss_Grunt"))
			{
				defenderWeight += DEFENDER_VALUE_BOSS;
			}
			else if (StringEqual(defenderClass, "Class_Lt_Grunt"))
			{
				defenderWeight += DEFENDER_VALUE_LT;
			}
			else
			{
				defenderWeight += DEFENDER_VALUE_MINION;
			}
		}
		*defenders = (NUMBER) defenderWeight;
	}

	// Count defenders close by, for sigil power computation
	if (powerDefenders != NULL)
	{
		defenderCount = GetAllEntities( entlist, 
									0,													// villain group
									0,													// villain def - Not yet implemented	
									0,													// villain tag - Not yet implemented
									0,													// ent type - Not yet implemented
									critterTeam,										// script team
									CRITTER_COUNT_RADIUS,								// radius to check 
									searchLocation,										// location to check
									0,													// area
									1,													// isAlive
									0,													// isDead
									MAXENTS,											// max ents
									0													// only once
									);

		*powerDefenders = defenderCount;
	}

	// Look for nearby Players
	if (players != NULL)
	{
		*players = GetAllEntities(entlist, 
									0,													// villain group
									0,													// villain def - Not yet implemented	
									0,													// villain tag - Not yet implemented
									0,													// ent type - Not yet implemented
									ALL_PLAYERS,										// script team
									PLAYER_INCLUDE_RADIUS,								// radius to check 
									searchLocation,										// location to check
									0,													// area
									1,													// isAlive
									0,													// isDead
									MAXENTS,											// max ents
									0													// only once
								);
	}
}

/////////////////////////////////////////////////////////////////////////
// Spawning
/////////////////////////////////////////////////////////////////////////

// Generic spawn function, about all this is used to do is spawn the debt protection critter
static void DeadlyApocalypseGenericSpawn(STRING layout, STRING marker, STRING team)
{ 
	ENCOUNTERGROUP group = "first";

	// spawn 
	group = FindEncounterGroupByRadius( 
		marker,
		0, 
		500, 
		layout, 
		1, 
		0 );  

	if (stricmp(group, "location:none") != 0) 
	{
		Spawn( 1, team, "UseCanSpawns", group, layout, 54, 0 );
	}
}

// Spawn defenders - this covers spawning all defenders.  Sigils and the final boss are handled elsewhere.
// loc is the center point of the spawn
// radius is the maximum distance away to spawn
// behavior is their initial behavior
// team is the team they're on
// teamCount is the number of critters to spwan
// spawnType says whether Bosses and/or EB's are allowed
// targetLoc is an optional place to run to after spawning -- deprecated
// wave is the sigil index, this is used to determine what groups to spawn from
static void DeadlyApocalypseSpawnDefenders(LOCATION loc, NUMBER radius, STRING behavior, TEAM team, NUMBER teamCount, NUMBER spawnType, 
											LOCATION targetLoc, int wave)
{
	int i;
	NUMBER currentDefenders;			// hard count of defenders
	int totalSpawned;					// total count we've spawned so far
	int rankRoll;						// random roll used to select rank
	int maximumRankRoll;				// Sum of ranges for rankRoll, i.e. that value will be in the range 0 .. maximumRankRoll - 1
	int spawnCheckCount;				// Safety check value to ensure we don't loop here forever trying to find a valid spawn location
	STRING minionList;					// List of minions we can spawn critters from
	STRING LTList;						// List of LTs we can spawn critters from
	STRING bossList;					// List of bossess we can spawn critters from
	STRING EBList;						// List of EBs we can spawn critters from
	STRING critterList;					// current critter list (based on rank)
	STRING critter;						// critter type we're going to spawn
	STRING spawnFX;						// spawn FX to play
	STRING currentCritter;				// critter we actually spawned
	NUMBER currentRadius;				// current spread radius used when spawning this critter
	Vec3 centerPos;						// Center of creation square
	Vec3 pos;							// Actual creation position
	Vec3 topPos;						// Straight up from pos, used in collision checking
	Vec3 adjCenterPos;					// Vertically above centerPos, but at the same altitude as the desired spawn point
	CollInfo coll;						// Opaque data structure - used in collision checking
	char critterLoc[256];				// String holding critter location
	char waveAsString[4];				// == NumberToString(wave)

	// Get and verify the center point of the spawn
	if (!GetPointFromLocation(loc, centerPos))
	{
		if (isDevelopmentMode())
		{
			assert(0 && "Bad location handed to DeadlyApocalypseSpawnDefenders(...)");
		}
		return;
	}

	// We've already got a guarantee that sigilCount < 10, therefore this single digit conversion will always work.
	waveAsString[0] = wave + '0';
	waveAsString[1] = 0;

	// Cook up the names of the critter lists that are active for this wave.
	// Bump the value encoded in waveAsString by 1 for the duration since these values are one based
	waveAsString[0]++;
	minionList = StringAdd("MinionList_", waveAsString);
	LTList = StringAdd("LTList_", waveAsString);
	bossList = StringAdd("BossList_", waveAsString);
	EBList = StringAdd("EBList_", waveAsString);
	spawnFX = VarGet(StringAdd("SpawnFX_", waveAsString));
	waveAsString[0]--;

	// Current count in 
	currentDefenders = NumEntitiesInTeam(team);

	totalSpawned = 0;

	switch (spawnType)
	{
		default:
		case SPAWN_LT_AND_MINION:
			maximumRankRoll = MINION_PROBABILITY + LT_PROBABILITY;
			break;
		case SPAWN_BOSS:
			maximumRankRoll = MINION_PROBABILITY + LT_PROBABILITY + BOSS_PROBABILITY;
			break;
		case SPAWN_EB:
			maximumRankRoll = MINION_PROBABILITY + LT_PROBABILITY + BOSS_PROBABILITY + EB_PROBABILITY;
			break;
	}
	// Loop till we either hit the hard ceiling (MAX_DEFENDERS), or the value of the defenders exceeds that of the players, or we hit the max allowed per spawn
	while (currentDefenders < MAX_DEFENDERS && totalSpawned < teamCount)
	{
		// Assume no legit options to begin with 
		critterList = NULL;
		// random roll for this guy's rank
		rankRoll = RandomNumber(0, maximumRankRoll - 1);
		// check if we selected a minion ...
		if ((rankRoll -= MINION_PROBABILITY) < 0)
		{
			critterList = minionList;
		}
		// or a lt ...
		else if ((rankRoll -= LT_PROBABILITY) < 0)
		{
			critterList = LTList;
		}
		// or a boss ...
		else if ((rankRoll -= BOSS_PROBABILITY) < 0)
		{
			critterList = bossList;
		}
		else
		{
			critterList = EBList;
		}
		if (!StringEmpty(critterList))
		{
			// figure the critter we're going to spawn
			critter = VarGetArrayElement(critterList, RandomNumber(0, VarGetArrayCount(critterList) - 1));

			// Now we've finally chosen the critter, see if there's an override spawn FX
			for (i = VarGetArrayCount("SpawnFXSpecialCaseCritters") - 1; i >= 0; i--)
			{
				if (StringEqual(critter, VarGetArrayElement("SpawnFXSpecialCaseCritters", i)))
				{
					spawnFX = VarGetArrayElement("SpawnFXSpecialCaseFX", i);
					break;
				}
			}

			currentRadius = radius;

			for (spawnCheckCount = 0; spawnCheckCount < 5; spawnCheckCount++)
			{
				// First things first.  Spread out around the spawn location, and then drop a probe to find the floor.  We scan +/- 20 ft
				// from the altitude of the spawn marker

				// /whisper Not really a radius: it's a square rather than a circle.
				// /whisper I won't tell if you don't.
				pos[0] = centerPos[0] + RandomFraction(-currentRadius, currentRadius);
				pos[1] = centerPos[1] - 20.0f;
				pos[2] = centerPos[2] + RandomFraction(-currentRadius, currentRadius);

				topPos[0] = pos[0];
				topPos[1] = pos[1] + 40.0f;
				topPos[2] = pos[2];

				if (collGrid(NULL, topPos, pos, &coll, 1.0f, COLL_NOTSELECTABLE|COLL_DISTFROMSTART|COLL_CYLINDER|COLL_BOTHSIDES))
				{
					// found the floor.  Adjust the position, and then do a LOS check to the ceiling
					pos[1] = coll.mat[3][1] + 0.1f;
					topPos[1] = pos[1] + 15.0f;
					// adjCenterPos is vertically aligned with centerpos, but at the same altitude as pos.
					adjCenterPos[0] = centerPos[0];
					adjCenterPos[1] = pos[1];
					adjCenterPos[2] = centerPos[2];

					if (collGrid(NULL, pos, topPos, &coll, 1.0f, COLL_NOTSELECTABLE|COLL_HITANY|COLL_CYLINDER|COLL_BOTHSIDES) == 0 /* && 
						collGrid(NULL, pos, adjCenterPos, &coll, 0.0f, COLL_NOTSELECTABLE|COLL_HITANY|COLL_BOTHSIDES) == 0 */)
					{
						// Good - there's room in here to stuff a 15 ft critter in (first check) and horizontal LOS from its feet
						// to the center x,z.  Spawn location is good - use it.
						break;
					}
				}
				if (spawnCheckCount != 0 && currentRadius > 10)
				{
					currentRadius -= 10;
				}
			}

			// If we actually did find a good spot, spawn the guy.
			if (spawnCheckCount < 5)
			{
				snprintf(critterLoc, sizeof(critterLoc), "coord:%f,%f,%f", pos[0], pos[1], pos[2]);
				critterLoc[sizeof(critterLoc) - 1] = 0;

				currentCritter = CreateVillain(team, critter, VarGetNumber("InvaderLevel"), NULL, critterLoc);
				if (!StringEqual(team, "Eventteam"))
				{
					SetScriptTeam(currentCritter, "EventTeam");
				}

				SetAIPriorityList(currentCritter, behavior);
				if (!StringEmpty(spawnFX))
				{
					ScriptPlayFXOnEntity(currentCritter, spawnFX);
				}
			}
			// However we count up even if the spawn failed.  If not, we could potentially get stuck here a bit too much
			currentDefenders++;
			totalSpawned++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Called immediately prior to spawning the sigil.  This just moves the players a little,
// and gives them some velocity.  The intent being to make sure they are not so close to
// the sigil that they actually get trapped in it's collision geometry
//
static void DeadlyApocalypseBumpPlayers(LOCATION location)
{
	ENTITY entlist[MAXENTS];
	NUMBER i;
	NUMBER numPlayers;
	ENTITY player;
	Vec3 locPos;
	Vec3 playerPos;
	Vec3 delta;
	float deltaLen;
	float sixOverDeltaLen;
	char coordBuff[256];

	if (!GetPointFromLocation(location, locPos))
	{
		//assert(0 && "Bad locatation handed to DeadlyApocalypseBumpPlayers(...)");
		return;
	}

	numPlayers = GetAllEntities(entlist, 
									0,					// villain group
									0,					// villain def - Not yet implemented	
									0,					// villain tag - Not yet implemented
									0,					// ent type - Not yet implemented
									ALL_PLAYERS,		// script team
									30.0f,				// radius to check 
									location,			// location to check
									0,					// area
									1,					// isAlive
									0,					// isDead
									MAXENTS,			// max ents
									0					// only once
								);



	for (i = 0; i < numPlayers; i++)
	{
		player = entlist[i];
		if (GetPointFromLocation(player, playerPos))
		{
			subVec3(playerPos, locPos, delta);
			deltaLen = distance3XZ(playerPos, locPos);
			if (deltaLen < 0.01f)
			{
				delta[0] = 6.0f;
				delta[2] = 0.0f;
			}
			else
			{
				sixOverDeltaLen = 6.0f / deltaLen;
				delta[0] *= sixOverDeltaLen;
				delta[2] *= sixOverDeltaLen;
			}
			if (delta[1] < 3.0f)
			{
				delta[1] = 3.0f;
			}
			else
			{
				delta[1] = 0.0f;
			}
			addVec3(playerPos, delta, playerPos);
			delta[1] = 3.0f;
			snprintf(coordBuff, sizeof(coordBuff), "coord:%f,%f,%f", playerPos[0], playerPos[1], playerPos[2]);
			coordBuff[sizeof(coordBuff) - 1] = 0;
			SetPosition(player, coordBuff);
			scaleByVec3(delta, 0.1f);
			ScriptApplyKnockBack(player, delta, 1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Called when anything defeats anything else.  This is used to check if all the defenders are dead,
// and if so, allow the players to attack the 
//
static void DeadlyApocalypseOnEntityDefeat(ENTITY player, ENTITY victor)
{
	int wave;
	int defenderTeamKills;
	int sigilCount;
	char waveAsString[4];

	sigilCount = VarGetNumber("SigilCount");
	waveAsString[1] = 0;
	for (wave = 0; wave < sigilCount; wave++)
	{
		waveAsString[0] = wave + '0';
		defenderTeamKills = VarGetArrayElementNumber("DefenderTeamKills", wave);
		// Count kills on the defender team
		if (IsEntityOnScriptTeam(player, StringAdd("WaveTeam.", waveAsString)))
		{
			defenderTeamKills++;
			VarSetArrayElementNumber("DefenderTeamKills", wave, defenderTeamKills);
		}

		// Check for the Sigil itself going down
		if (StringEqual(player, VarGet(StringAdd("Sigil_", waveAsString))))
		{
			VarSetArrayElementNumber("SigilDead", wave, 1);
			VarSetNumber(StringAdd("IconState_", waveAsString), 0);

			// remove map marker
			if (sigilMapMarkers[wave] != 0)
			{
				DestroyWaypoint(sigilMapMarkers[wave]);
				sigilMapMarkers[wave] = 0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Called when a player enters the map - set up the waypoint markers for the Sigils
//
int DeadlyApocalypseOnEnterMap(ENTITY player)
{
	int i;
	int count = VarGetNumber("SigilCount");

	// Set waypoints correctly
	for (i = 0; i < count; i++)
	{
		if (sigilMapMarkers[i] != 0)
		{
			SetWaypoint(player, sigilMapMarkers[i]);
		}
	}

	SetScriptTeam(player, "NoSigilArea");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// Called when a player leaves the map - clear any minimap waypoints that might be set
//
int DeadlyApocalypseOnLeaveMap(ENTITY player)
{
	// remove waypoints
	ClearAllWaypoints(player);

	LeaveAllScriptTeams(player);

	ScriptUIHide("Collection:SigilUI", player);
	VarSet(EntVar(player, "UITimer"), NULL);

	return 0;
}

static void DeadlyApocalypseResetTimers(void)
{
	TimerRemove("StateTimer");
	TimerRemove("SigilTimer");
	TimerRemove("PrepareTimer");
	TimerRemove("PowerTimer");
	TimerRemove("BossUITimer");
	TimerRemove("ApocalypseTimer");
	TimerRemove("PrepareTimer");
}

static void DeadlyApocalypseShutdown(void)
{
	// allow zone spawns
	// SetDisableMapSpawns(false);

	ClearAllWaypoints(ALL_PLAYERS);
	Kill("EventTeam", false);
	Kill("Debt", false);
	EndScript();
}

void DeadlyApocalypse(void)
{
	int i;
	int temp;
	int swap;
	//NUMBER unpathedCount;
	NUMBER wave;
	NUMBER waveTeamCount;
	STRING waveTeam;
	ENTITY currentCritter;
	STRING powerList;
	NUMBER powerCount;
	NUMBER requiredCritterCount;
	NUMBER sigilCount;
	NUMBER sigilLocations[MAX_WAVE];
	STRING currentSigilName;
	NUMBER currentSigilLocation;
	STRING currentSigilLocationStr;
	STRING currentSigilMapIcon;
	NUMBER currentAmbushIndex;
	STRING currentAmbushLocationStr;
	NUMBER sigilDead;
	NUMBER defenderTeamKills;
	NUMBER defenderTotalKills;
	NUMBER bossLocationIndex;
	NUMBER bossCount;
	NUMBER bossIndex;
	STRING bossName;
	STRING bossLocationStr;
	ENTITY currentBoss;
	NUMBER waveOrder[MAX_WAVE];
	ENTITY currentSigil;
	NUMBER playerCount;
	NUMBER defenderCount;
	NUMBER realDefenderCount;
	NUMBER targetTeamCount;
	NUMBER spawnType;
	ENTITY currentPlayer;
	STRING currentTeam;
	STRING currentUI;
	STRING sigilPower;
	STRING previousSigilPower;
	STRING sigilPowerName;
	STRING sigilDisplayName[MAX_WAVE];
	STRING targetPos;
	NUMBER UIHijackTime;
	FRACTION distanceToBoss;
	//FRACTION bossHealth;
	//FRACTION minimumHealth;
	char waveAsString[4];
	char templateResult[TEMPLATE_RESULT_SIZE];

	INITIAL_STATE      
	//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  

			// if there are too many running, don't spawn another
			if (ScriptCountRunningInstances("DeadlyApocalypse") > 1)
			{
				EndScript();
				return;
			}

			VarSetNumber("ApocalypseSky", SkyFileGetByName("Sun_Halloween_Deadly.txt"));

			for (i = 0; i < MAX_WAVE; i++)
			{
				sigilMapMarkers[i] = 0;
			}

			// We can't create the icon widget yet, because that requires the display names of the Sigils.  We don't get access to
			// those names until we actually place them.  So I'm faced with two options.  1. Create and destroy the four sigils here
			// just to get to their names, or 2. Delay creating this widget till the first time we actually place the sigils.  I
			// opt for option 2 since it's a lot less code.
			// Set a flag to note that we need to make the UI
			VarSetNumber("NeedToCreateUI", 1);
			VarSetNumber("KillProduct", 0);
			VarSetNumber("IconState_0", 1);
			VarSetNumber("IconState_1", 1);
			VarSetNumber("IconState_2", 1);
			VarSetNumber("IconState_3", 1);
			VarSet("SigilPower_0", "None");
			VarSet("SigilPower_1", "None");
			VarSet("SigilPower_2", "None");
			VarSet("SigilPower_3", "None");
			VarSet("CurrentUI", "");

			OnEntityDefeated(DeadlyApocalypseOnEntityDefeat);
			OnPlayerExitingMap(DeadlyApocalypseOnLeaveMap);
			OnPlayerEnteringMap(DeadlyApocalypseOnEnterMap);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		VarSetNumber("FirstTime", 1);
		SET_STATE("WaitingToStart");

	//////////////////////////////////////////////////////////////////////////////////
	STATE("WaitingToStart") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// happy sunny skies
			if (VarGetNumber("ApocalypseSky") >= 0)
			{
				SkyFileFade(VarGetNumber("ApocalypseSky"), 0, 1.0f); 
			}

			// people are happy
			SetAreAllNPCsScared(false);

			// pick time for next invasion
			VarSetFraction("NextInvasion", RandomFraction(VarGetNumber("FirstTime") ? 15.0f : VarGetFraction("MinInvasion"), 
															VarGetFraction("MaxInvasion")));
			VarSetNumber("FirstTime", 0);

			Kill("Debt", false); 
			Kill("EventTeam", false);
			Kill("SigilTeam", false);

			ClearAllWaypoints(ALL_PLAYERS);

			// reset timers
			DeadlyApocalypseResetTimers();

			// allow zone spawns
			// SetDisableMapSpawns(false);

			if (VarGetNumber("IHaveMutex"))
			{
				// release the mutex
				InstanceVarRemove("ZoneEventMutex");
				VarSetNumber("IHaveMutex", 0);
			}

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// Check 
		if (DoEvery("StateTimer", VarGetFraction("NextInvasion"), NULL))
		{			
			Kill("EventTeam", false);
			Kill("SigilTeam", false);
			SET_STATE("OneTimePause");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("OneTimePause") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all defenders are dead!
			Kill("EventTeam", false);
			Kill("SigilTeam", false);

			// reset timers
			DeadlyApocalypseResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			// if the world event is running, move immediately to the prelude
			if (MapGetDataToken(VarGet("DeadlyEventToken")) > 0)
			{
				SET_STATE("Prelude");

				// grab the mutex
				InstanceVarSet("ZoneEventMutex", "Sigil");
				VarSetNumber("IHaveMutex", 1);
			}
			else
			{
				// if the world event isn't running, check for the one time invasions only every minute
				if (DoEvery("StateTimer", STATE_CHANGE_DELAY, NULL))
				{			
					if (MapGetDataToken(VarGet("DeadlyOneTimeToken")) > 0)
					{
						MapRemoveDataToken(VarGet("DeadlyOneTimeToken"));
						SET_STATE("Prelude");

						// grab the mutex
						InstanceVarSet("ZoneEventMutex", "Sigil");
						VarSetNumber("IHaveMutex", 1);
					}
				}
			}
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("RunMe") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Be Certain all defenders are dead!
			Kill("EventTeam", false);
			Kill("SigilTeam", false);

			// reset timers
			DeadlyApocalypseResetTimers();
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// if some other invasion is running, just wait
		if (InstanceVarIsEmpty("ZoneEventMutex"))
		{
			SET_STATE("Prelude");

			// grab the mutex
			VarSetNumber("IHaveMutex", 1);
			InstanceVarSet("ZoneEventMutex", "Sigil");
		}
		else
		{
			SET_STATE("WaitingToStart");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Prelude") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// darken sky
			if (VarGetNumber("ApocalypseSky") >= 0)
			{
				SkyFileFade(0, VarGetNumber("ApocalypseSky"), 1.0f); 
			}

			// reset timers
			DeadlyApocalypseResetTimers();

			// send alert
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("AlertMessage"), 1,
																"zonename", StringLocalizeMenuMessages(GetMapName())));

			SendPlayerSound(ALL_PLAYERS, VarGet("AlertSound"), SOUND_MUSIC, 1.0f); 

			SetAreAllNPCsScared(true);

			// spawn in debt protector
			DeadlyApocalypseGenericSpawn("Hospital", "marker:Hospital", "Debt");

			// disallow zone spawns
			// SetDisableMapSpawns(true);

		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		if (DoEvery("StateTimer", STATE_CHANGE_DELAY, NULL))
		{			
			SET_STATE("Sigils");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Sigils") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY
			// Read in the tuning values from the scriptdef.  Read these each time we spawn the sigils, provides fastest iteration possible.
			MAX_DEFENDERS = VarGetNumber("MaxDefenders");
			MAX_DEFENDER_WANDER = VarGetNumber("MaxDefenderWander");
			DEFENDER_KILLS_NEEDED = VarGetNumber("DefenderKillsNeeded");
			ENABLE_SPAWNS_AT_ASPECT = MapGetDataToken("EnableSpawnsAtAspect");
			if (ENABLE_SPAWNS_AT_ASPECT < 0 || ENABLE_SPAWNS_AT_ASPECT > 4)
			{
				ENABLE_SPAWNS_AT_ASPECT = 0;
			}
			THROTTLE_ASPECT_DAMAGE = MapGetDataToken("ThrottleAspectDamage");
			if (THROTTLE_ASPECT_DAMAGE < 0 || THROTTLE_ASPECT_DAMAGE > 10)
			{
				THROTTLE_ASPECT_DAMAGE = 0;
			}

			// reset timers
			DeadlyApocalypseResetTimers(); 

			sigilCount = VarGetNumber("SigilCount");

			// This is a hard requirement since it forces the sigil indices always to be a single digit.
			// See the generation of waveAsString a few lines further down for the reason why.
			STATIC_INFUNC_ASSERT(MAX_WAVE < 10);

			// cap by the smaller of SigilLocationCount and MAX_WAVE
			if (SigilLocationCount < MAX_WAVE)
			{
				if (sigilCount > SigilLocationCount)
				{
					sigilCount = SigilLocationCount;
					VarSetNumber("SigilCount", sigilCount);
				}
			}
			else
			{
				if (sigilCount > MAX_WAVE)
				{
					sigilCount = MAX_WAVE;
					VarSetNumber("SigilCount", sigilCount);
				}
			}

			if (sigilCount != 4)
			{
				// if sigilCount is ever not equal to 4, this will do weird stuff.  The bar movement will be a bit funky.

				// Evaluate final kill product, now that we have sigilCount correctly figured
				DEFENDER_KILLS_NEEDED = cappedIntegerPower(DEFENDER_KILLS_NEEDED, sigilCount, 2000000000);

				// Sneaky way to take the 4th root of something, fsqrt it twice.  Find the other fsqrt's in here for why.
				DEFENDER_KILLS_NEEDED = (int) fsqrt(fsqrt((float) DEFENDER_KILLS_NEEDED));
			}

			// And drop into a variable for the UI
			VarSetNumber("KillProductNeeded", DEFENDER_KILLS_NEEDED);

			// Randomize the power arrangement using a classic Fisher-Yates shuffle
			for (i = 0; i < sigilCount; i++)
			{
				// +1 because the values in the scriptdef file are one based.
				waveOrder[i] = i + 1;
			}
			for (i = sigilCount - 1; i >= 1; i--)
			{
				swap = RandomNumber(0, i);
				temp = waveOrder[swap];
				waveOrder[swap] = waveOrder[i];
				waveOrder[i] = temp;
			}
			// Save the power order
			VarSet("PowerIndices", "");
			for (i = 0; i < sigilCount; i++)
			{
				VarSetArrayElementNumber("PowerIndices", i, waveOrder[i]);
				VarSetArrayElementNumber("SigilDead", i, 0);
				VarSetArrayElementNumber("DefenderTeamKills", i, 0);
			}

			// generate the sigil locations
			for (wave = 0; wave < sigilCount; wave++)
			{
				// We've already got a guarantee that sigilCount < 10, therefore this single digit conversion will always work.
				waveAsString[0] = wave + '0';
				waveAsString[1] = 0;
				do
				{
					// Pick a random location
					sigilLocations[wave] = RandomNumber(1, SigilLocationCount);
					for (i = 0; i < wave; i++)
					{
						// check for previous usage
						if (sigilLocations[wave] == sigilLocations[i])
						{
							break;
						}
					}
				// Repeat till we get a new one
				} while (i < wave);

				snprintf(templateResult, TEMPLATE_RESULT_SIZE, SigilLocationTemplate, sigilLocations[wave]);
				templateResult[TEMPLATE_RESULT_SIZE - 1] = 0;
				currentSigilLocationStr = StringAdd("marker:", templateResult);
				VarSet(StringAdd("SigilLocations_", waveAsString), currentSigilLocationStr);
				VarSetArrayElementNumber("SigilLocations", wave, sigilLocations[wave]);

				// Spawn the sigil
				// Bump the value in WaveAsString by one since SigilName_X and SigilMapIcon_X values are one based.
				waveAsString[0]++;
				currentSigilName = VarGet(StringAdd("SigilName_", waveAsString));
				currentSigilMapIcon = VarGet(StringAdd("SigilMapIcon_", waveAsString));
				waveAsString[0]--;
				DeadlyApocalypseBumpPlayers(currentSigilLocationStr);
				currentSigil = CreateVillain("SigilTeam", currentSigilName, 0, NULL, currentSigilLocationStr);
				VarSet(StringAdd("Sigil_", waveAsString), currentSigil);

				sigilDisplayName[wave] = GetDisplayName(currentSigil);
				sigilMapMarkers[wave] = CreateWaypoint(currentSigilLocationStr, sigilDisplayName[wave], 
													currentSigilMapIcon, NULL, false, false, -1.0f);
				if (sigilMapMarkers[wave] > 0)
				{
					SetWaypoint(ALL_PLAYERS, sigilMapMarkers[wave]);
				}

				// Set the Sigil intangible and unselectable  for now
				SetAIPriorityList(currentSigil, SIGIL_INVULNERABLE_PL);
				// And turn on the transparency toggle to guive the players a visual cue
				EntityActivatePower(currentSigil, NULL, VarGet("SigilTranslucentPower"));
			}
			if (VarGetNumber("NeedToCreateUI"))
			{
				// We'll only ever hit this once, the first time through.  Now that we have the sigil display names, we can
				// create the UI
				ScriptUITitle("TitleWidget", "TitleText", "TitleHint");
				ScriptUITimer("TimerWidget", "ApocalypseTimerUI", VarGet("SigilVulnerability"), "");
				ScriptUITimer("TimerWidget2", "ApocalypseTimerUI", VarGet("DestroySigils"), "");
				ScriptUITimer("TimerWidget3", "ApocalypseTimerUI", VarGet("DestroyBoss"), "");
				ScriptUIMeter("BarWidget", "KillProduct", "", "0", "0", "KillProductNeeded", "2255ffff", "dd0000ff", "");
				ScriptUICollectItems("IconWidget", 4, "IconState_0", sigilDisplayName[0], VarGet("SigilUIIcon_1"),
													  "IconState_1", sigilDisplayName[1], VarGet("SigilUIIcon_2"),
													  "IconState_2", sigilDisplayName[2], VarGet("SigilUIIcon_3"),
													  "IconState_3", sigilDisplayName[3], VarGet("SigilUIIcon_4"));
				ScriptUIDistance("DistanceWidget", "", VarGet("BossDist"), "ffffffff", "BossCoords", "ffffffff");

				ScriptUICreateCollection("SigilUIInv", 4, "TitleWidget", "TimerWidget", "BarWidget", "IconWidget");
				ScriptUICreateCollection("SigilUIVulnerable", 3, "TitleWidget", "TimerWidget2", "IconWidget");
				ScriptUICreateCollection("SigilUIBoss", 3, "TitleWidget", "TimerWidget3", "DistanceWidget");
				ScriptUICreateCollection("SigilUI", 7, "TitleWidget", "TimerWidget", "TimerWidget2", "TimerWidget3", "BarWidget", "IconWidget", "DistanceWidget");
				VarSetNumber("NeedToCreateUI", 0);
			}

			VarSetNumber("WaveCheck", 0);
			VarSetNumber("SigilsVulnerable", 0);
			VarSetNumber("UICheck", 0);
			VarSetNumber("Preparing", 1);
			TimerSet("PrepareTimer", (FRACTION) VarGetNumber("SigilPrepareTime"));
			VarSetNumber("KillProduct", 0);
			VarSetNumber("IconState_0", 1);
			VarSetNumber("IconState_1", 1);
			VarSetNumber("IconState_2", 1);
			VarSetNumber("IconState_3", 1);
			VarSet("SigilPower_0", "None");
			VarSet("SigilPower_1", "None");
			VarSet("SigilPower_2", "None");
			VarSet("SigilPower_3", "None");

			// Reset teamns that control the UI
			SwitchScriptTeam("SigilPerm", "SigilPerm", NULL);
			SwitchScriptTeam("SigilArea.0", "SigilArea.0", NULL);
			SwitchScriptTeam("SigilArea.1", "SigilArea.1", NULL);
			SwitchScriptTeam("SigilArea.2", "SigilArea.2", NULL);
			SwitchScriptTeam("SigilArea.3", "SigilArea.3", NULL);
			SetScriptTeam(ALL_PLAYERS, "NoSigilArea");

			VarSet("CurrentUI", "Collection:SigilUIInv");

			TimerSet("ApocalypseTimer", VarGetNumber("SigilDuration"));
			VarSetNumber("ApocalypseTimerUI", (int) (VarGetNumber("SigilDuration") * 60 + timerSecondsSince2000()));
		END_ENTRY

		// Need to do this again because it's possible to get here without having done the ON_ENTRY block
		sigilCount = VarGetNumber("SigilCount");

		// Fire these every 4 seconds.
		if (VarGetNumber("Preparing") == 0 && DoEvery("PowerTimer", (4.0f / (float) sigilCount) / 60.0f, NULL))
		{
			wave = VarGetNumber("WaveCheck");
			sigilDead = VarGetArrayElementNumber("SigilDead", wave);

			if (sigilDead == 0)
			{
				// We've already got a guarantee that sigilCount < 10, therefore this single digit conversion will always work.
				waveAsString[0] = wave + '0';
				waveAsString[1] = 0;

				waveTeam = StringAdd("WaveTeam.", waveAsString);
				waveTeamCount =  NumEntitiesInTeam(waveTeam);
				currentSigil = VarGet(StringAdd("Sigil_", waveAsString));
				currentSigilLocationStr = VarGet(StringAdd("SigilLocations_", waveAsString));

				// Now check for any defenders that have wandered to far and force them to run back here
				for (i = 0; i < waveTeamCount; i++)
				{
					currentCritter = GetEntityFromTeam(waveTeam, i + 1);
					if (!CurAIBehaviorMatchesName(currentCritter, "MoveToPos") && DistanceBetweenEntities(currentCritter, currentSigil) > (float) MAX_DEFENDER_WANDER)
					{
						targetPos = GetSafeTargetPos(currentSigilLocationStr, FINAL_RETURN_RADIUS);
						if (!StringEqual(targetPos, LOCATION_NONE))
						{
							AddAIBehavior(currentCritter, "Combat");
							SetAIMoveTarget(currentCritter, targetPos, HIGH_PRIORITY, 0, 1, 0.0f);
						}
					}
				}

				// Do we need to spawn more defenders?
				DeadlyApocalypseCountEnts(&playerCount, &defenderCount, &realDefenderCount, wave);

				if (playerCount > 6 * defenderCount && defenderCount > 0)
				{	
					// dedfenders are vastly out-numbered
					targetTeamCount = MIN(8, playerCount - defenderCount) / 3;
					spawnType = SPAWN_EB;
				} 
				else if (playerCount > 3 * defenderCount && defenderCount > 0)
				{	
					// defenders are out-numbered
					targetTeamCount = MIN(8, playerCount - defenderCount) / 2;
					spawnType = SPAWN_BOSS;
				}
				else if (playerCount > defenderCount)
				{
					// defenders are out-gunned
					targetTeamCount = MIN(8, playerCount - defenderCount);
					spawnType = SPAWN_LT_AND_MINION;
				}
				else
				{
					targetTeamCount = 0;
				}

				if (targetTeamCount)
				{
					// Overall, if there's too few defenders, spawn a bunch more
					// Build the spawn location from the current sigil location and a random selector from the associated ambush spawn points
					currentSigilLocation = VarGetArrayElementNumber("SigilLocations", wave);
					currentAmbushIndex = RandomNumber(1, AmbushLocationCount);

					snprintf(templateResult, TEMPLATE_RESULT_SIZE, AmbushLocationTemplate, currentSigilLocation, currentAmbushIndex);
					templateResult[TEMPLATE_RESULT_SIZE - 1] = 0;

					currentAmbushLocationStr = StringAdd("marker:", templateResult);

					DeadlyApocalypseSpawnDefenders(VarGet(StringAdd("SigilLocations_", waveAsString)), FINAL_AMBUSH_RADIUS, DEFENDER_SPAWN_PL, StringAdd("WaveTeam.", waveAsString), targetTeamCount, 
											spawnType, NULL , wave);
				}

				if (VarGetNumber("SigilsVulnerable") == 1)
				{
					// These are really the names of the variables that hold the payload we care about
					i = VarGetArrayElementNumber("PowerIndices", wave);
					powerList = StringAdd("PowerList_", NumberToString(i));

					// And the number of powers in the list
					powerCount = VarGetArrayCount(powerList) - 1;
					// Except clamp it by the number of ratios ...
					i = VarGetArrayCount("PowerActivationCounts");
					if (powerCount > i)
					{
						powerCount = i;
					}

					for (i = 0; i < powerCount; i++)
					{
						// Get the target count
						requiredCritterCount = VarGetArrayElementNumber("PowerActivationCounts", i);
						// If target count is greater than actual count of critters ...
						if (requiredCritterCount > realDefenderCount)
						{
							// we'll use this power
							break;
						}
					}
					// Go get the power
					sigilPower = VarGetArrayElement(powerList, i);
					sigilPowerName = StringAdd("SigilPower_", waveAsString);
					previousSigilPower = VarGet(sigilPowerName);
					if (!StringEqual(sigilPower, previousSigilPower))
					{
						// Was there previously a power on this guy?
						if (!StringEqual(previousSigilPower, "None"))
						{
							EntityDeactivateTogglePower(VarGet(StringAdd("Sigil_", waveAsString)), previousSigilPower);
						}

						// Is there one to add now?
						if (!StringEqual(sigilPower, "None"))
						{
							// Activate it.
							EntityActivatePower(VarGet(StringAdd("Sigil_", waveAsString)), NULL, sigilPower);
						}
						VarSet(sigilPowerName, sigilPower);
					}
				}
			}
			if (++wave >= sigilCount)
			{
				wave = 0;
			}
			VarSetNumber("WaveCheck", wave);

			if (VarGetNumber("SigilsVulnerable") == 0)
			{
				defenderTotalKills = 1;
				for (wave = 0; wave < sigilCount; wave++)
				{
					defenderTeamKills = VarGetArrayElementNumber("DefenderTeamKills", wave);
					defenderTotalKills = defenderTotalKills * defenderTeamKills;
				}

				// OK, we now fsqrt this twice, as a way to take the 4th root of it.  This makes the bar movement
				// far more linear as far as the kill count product is concerned.
				defenderTotalKills = (int) fsqrt(fsqrt((float) defenderTotalKills));

				if (defenderTotalKills >= DEFENDER_KILLS_NEEDED)
				{
					defenderTotalKills = DEFENDER_KILLS_NEEDED;
				}

				VarSetNumber("KillProduct", defenderTotalKills);

				if (defenderTotalKills >= DEFENDER_KILLS_NEEDED)
				{
					VarSetNumber("SigilsVulnerable", 1);
					for (wave = 0; wave < sigilCount; wave++)
					{
						// We've already got a guarantee that sigilCount < 10, therefore this single digit conversion will always work.
						waveAsString[0] = wave + '0';
						waveAsString[1] = 0;

						currentSigil = VarGet(StringAdd("Sigil_", waveAsString));
						// Allow players to attack the sigil
						SetAIPriorityList(currentSigil, SIGIL_VULNERABLE_PL);
						// Turn off the transparency toggle
						EntityDeactivateTogglePower(currentSigil, VarGet("SigilTranslucentPower"));

						currentTeam = StringAdd("SigilArea.", waveAsString);
						// Having this line here limits the display of the banners vulnerable floater to just the proximity of the banners
						//SendPlayerAttentionMessageWithColor(currentTeam, VarGet("SigilVulnerable"), 0x00ffffff, kFloaterStyle_Attention);
						ScriptUIHide("Collection:SigilUIInv", currentTeam);
						ScriptUIShow("Collection:SigilUIVulnerable", currentTeam);
					}
					// And switch the ui for the permahijack folks
					ScriptUIHide("Collection:SigilUIInv", "SigilPerm");
					ScriptUIShow("Collection:SigilUIVulnerable", "SigilPerm");

					// Doing this here rather than up above sends the "Banners Vulnerable" message zonewide
					SendPlayerAttentionMessageWithColor(ALL_PLAYERS, VarGet("SigilVulnerable"), 0x00ffffff, kFloaterStyle_Attention);
					VarSetNumber("IconState_0", 1);
					VarSetNumber("IconState_1", 1);
					VarSetNumber("IconState_2", 1);
					VarSetNumber("IconState_3", 1);
					VarSet("CurrentUI", "Collection:SigilUIVulnerable");
				}
			}
		}

		i = 0;
		for (wave = 0; wave < sigilCount; wave++)
		{
			if (VarGetArrayElementNumber("SigilDead", wave) != 0)
			{
				i++;
			}
		}

		if (i == sigilCount)
		{
			// These four lines just spam the message to players in proximity to the banners
			//SendPlayerAttentionMessageWithColor("SigilArea.0", VarGet("SigilDefeated"), 0x00ffffff, kFloaterStyle_Attention);
			//SendPlayerAttentionMessageWithColor("SigilArea.1", VarGet("SigilDefeated"), 0x00ffffff, kFloaterStyle_Attention);
			//SendPlayerAttentionMessageWithColor("SigilArea.2", VarGet("SigilDefeated"), 0x00ffffff, kFloaterStyle_Attention);
			//SendPlayerAttentionMessageWithColor("SigilArea.3", VarGet("SigilDefeated"), 0x00ffffff, kFloaterStyle_Attention);
			// While this line sends to all players
			SendPlayerAttentionMessageWithColor(ALL_PLAYERS, VarGet("SigilDefeated"), 0x00ffffff, kFloaterStyle_Attention);

			// Adjust UI
			for (wave = 0; wave < sigilCount; wave++)
			{
				waveAsString[0] = wave + '0';
				waveAsString[1] = 0;

				// get the current location  and current team
				currentSigilLocationStr = VarGet(StringAdd("SigilLocations_", waveAsString));
				currentTeam = StringAdd("SigilArea.", waveAsString);

				for (i = NumEntitiesInTeam(currentTeam); i > 0; i--)
				{
					currentPlayer = GetEntityFromTeam(currentTeam, i);
					ScriptUIHide("Collection:SigilUIVulnerable", currentPlayer);
					// We now check how long they were in range, if it's greater than the hijack duration, we perma hijack their UI
					UIHijackTime = VarGetNumber(EntVar(currentPlayer, "UITimer"));
					if (UIHijackTime > 0 && UIHijackTime + VarGetNumber("UIHijackTime") < (int) timerSecondsSince2000())
					{
						SwitchScriptTeam(currentPlayer, currentTeam, "SigilPerm");
						ScriptUIShow("Collection:SigilUIBoss", currentPlayer);
					}
					else
					{
						SwitchScriptTeam(currentPlayer, currentTeam, "NoSigilArea");
					}
				}
			}
			// And switch the ui for the permahijack folks
			ScriptUIHide("Collection:SigilUIVulnerable", "SigilPerm");
			ScriptUIShow("Collection:SigilUIBoss", "SigilPerm");
			VarSet("CurrentUI", "Collection:SigilUIBoss");
			SET_STATE("BossFight");
		}
		else
		{
			currentUI = VarGet("CurrentUI");
			// wave counts [0 .. sigilCount * 2)
			wave = VarGetNumber("UICheck");

			// Need to mod sigilCount because we count twice as far
			waveAsString[0] = wave % sigilCount + '0';
			waveAsString[1] = 0;

			// get the current location  and current team
			currentSigilLocationStr = VarGet(StringAdd("SigilLocations_", waveAsString));
			currentTeam = StringAdd("SigilArea.", waveAsString);

			if (wave < sigilCount)
			{
				// The first half [0 .. sigilCount) we check if anyone who was in range has moved too far away
				i = NumEntitiesInTeam(currentTeam);
				while (i > 0)
				{
					currentPlayer = GetEntityFromTeam(currentTeam, i);
					if (DistanceBetweenLocations(PointFromEntity(currentPlayer), currentSigilLocationStr) > UI_DISAPPEAR_RADIUS)
					{
						// OK, this player has moved away.  We now check how long they were in range, if it's greater than the hijack duration, we perma hijack their UI
						UIHijackTime = VarGetNumber(EntVar(currentPlayer, "UITimer"));
						if (UIHijackTime > 0 && UIHijackTime + VarGetNumber("UIHijackTime") < (int) timerSecondsSince2000())
						{
							SwitchScriptTeam(currentPlayer, currentTeam, "SigilPerm");
						}
						else
						{
							SwitchScriptTeam(currentPlayer, currentTeam, "NoSigilArea");
							ScriptUIHide(currentUI, currentPlayer);
						}
					}
					i--;
				}
			}
			else
			{
				// The second half [sigilCount .. sigilCount * 2) we check if anyone who was too far away has moved into range
				waveAsString[0] = (wave - sigilCount) + '0';
				waveAsString[1] = 0;

				currentSigilLocationStr = VarGet(StringAdd("SigilLocations_", waveAsString));
				currentTeam = StringAdd("SigilArea.", waveAsString);

				i = NumEntitiesInTeam("NoSigilArea");
				while (i > 0)
				{
					currentPlayer = GetEntityFromTeam("NoSigilArea", i);
					if (DistanceBetweenLocations(PointFromEntity(currentPlayer), currentSigilLocationStr) < UI_APPEAR_RADIUS)
					{
						SwitchScriptTeam(currentPlayer, "NoSigilArea", currentTeam);
						ScriptUIShow(currentUI, currentPlayer);
						VarSetNumber(EntVar(currentPlayer, "UITimer"), timerSecondsSince2000());
					}
					i--;
				}
			}
			if (++wave >= sigilCount * 2)
			{
				wave = 0;
			}
			VarSetNumber("UICheck", wave);
		}

		if (TimerElapsed("PrepareTimer"))
		{
			VarSetNumber("Preparing", 0);
			TimerRemove("PrepareTimer");
		}

		if (TimerElapsed("ApocalypseTimer"))
		{
			SET_STATE("Epilogue");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("BossFight") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			bossLocationIndex = RandomNumber(1, BossLocationCount); 
			snprintf(templateResult, TEMPLATE_RESULT_SIZE, BossLocationTemplate, bossLocationIndex);
			templateResult[TEMPLATE_RESULT_SIZE - 1] = 0;

			bossCount = VarGetArrayCount("FinalBossList");
			bossIndex = RandomNumber(0, bossCount - 1);
			bossName = VarGetArrayElement("FinalBossList", bossIndex);

			bossLocationStr = StringAdd("marker:", templateResult);

			currentBoss = CreateVillain("BossTeam", bossName, 0, NULL, bossLocationStr);
			SetScriptTeam(currentBoss, "EventTeam");
			VarSet("FinalBoss", currentBoss);
			VarSet("BossLocation", bossLocationStr);
			VarSetNumber("UIPhase", 0);
			VarSetNumber("BossFirstBlood", 0);

			VarSet("BossCoords", GetXYZStringFromLocation(bossLocationStr));

			SetAIPriorityList("EventTeam", "Combat");

			TimerSet("ApocalypseTimer", VarGetNumber("BossDuration"));
			VarSetNumber("ApocalypseTimerUI", (int) (VarGetNumber("BossDuration") * 60 + timerSecondsSince2000()));
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		bossLocationStr = VarGet("BossLocation");
		if (DoEvery("BossUITimer", 1.0f / 60.0f, NULL))
		{
			currentUI = VarGet("CurrentUI");

			if (VarGetNumber("UIPhase") == 0)
			{
				for (i = NumEntitiesInTeam("BossLocTeam"); i > 0; i--)
				{
					currentPlayer = GetEntityFromTeam("BossLocTeam", i);
					distanceToBoss = DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr);
					if (DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr) > UI_DISAPPEAR_RADIUS)
					{
						// OK, this player has moved away.  We now check how long they were in range, if it's greater than the hijack duration, we perma hijack their UI
						UIHijackTime = VarGetNumber(EntVar(currentPlayer, "UITimer"));
						if (UIHijackTime > 0 && UIHijackTime + VarGetNumber("UIHijackTime") < (int) timerSecondsSince2000())
						{
							SwitchScriptTeam(currentPlayer, "BossLocTeam", "SigilPerm");
						}
						else
						{
							SwitchScriptTeam(currentPlayer, "BossLocTeam", "NoSigilArea");
							ScriptUIHide(currentUI, currentPlayer);
						}
					}
				}
				VarSetNumber("UIPhase", 1);
			}
			else
			{
				for (i = NumEntitiesInTeam("NoSigilArea"); i > 0; i--)
				{
					currentPlayer = GetEntityFromTeam("NoSigilArea", i);
					distanceToBoss = DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr);
					if (DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr) < UI_APPEAR_RADIUS)
					{
						SwitchScriptTeam(currentPlayer, "NoSigilArea", "BossLocTeam");
						ScriptUIShow(currentUI, currentPlayer);
						VarSetNumber(EntVar(currentPlayer, "UITimer"), timerSecondsSince2000());
					}
				}
				VarSetNumber("UIPhase", 1);
			}
		}

#if 0
		// Save this lot for playtest after the dxp weekend
		if (ENABLE_SPAWNS_AT_ASPECT && DoEvery("PowerTimer", 0.1f, NULL))
		{
			// Do we need to spawn more defenders?
			DeadlyApocalypseCountEnts(&playerCount, &defenderCount, &realDefenderCount, -1);

			if (playerCount > 6 * defenderCount && defenderCount > 0)
			{	
				// dedfenders are vastly out-numbered
				targetTeamCount = MIN(8, playerCount - defenderCount) / 3;
				spawnType = SPAWN_EB;
			} 
			else if (playerCount > 3 * defenderCount && defenderCount > 0)
			{	
				// defenders are out-numbered
				targetTeamCount = MIN(8, playerCount - defenderCount) / 2;
				spawnType = SPAWN_BOSS;
			}
			else if (playerCount > defenderCount)
			{
				// defenders are out-gunned
				targetTeamCount = MIN(8, playerCount - defenderCount);
				spawnType = SPAWN_LT_AND_MINION;
			}
			else
			{
				targetTeamCount = 0;
			}

			if (targetTeamCount)
			{
				// Overall, if there's too few defenders, spawn a bunch more
				sigilCount = VarGetNumber("SigilCount");

				DeadlyApocalypseSpawnDefenders(VarGet("BossLocation"), FINAL_AMBUSH_RADIUS, DEFENDER_SPAWN_PL, "EventTeam", targetTeamCount, 
											spawnType, NULL, RandomNumber(0, sigilCount - 1));
			}
		}
		if (THROTTLE_ASPECT_DAMAGE)
		{
			currentBoss = VarGet("FinalBoss");
			bossHealth = GetHealth(currentBoss);

			if (VarGetNumber("BossFirstBlood") == 0)
			{
				if (bossHealth < 1.0f)
				{
					VarSetNumber("BossFirstBlood", timerSecondsSince2000());
				}
			}
			else
			{
				minimumHealth = 1.0f - (float) (timerSecondsSince2000() - VarGetNumber("BossFirstBlood")) / (THROTTLE_ASPECT_DAMAGE * 60.0f);
				if (minimumHealth < 1.0f && minimumHealth > 0.05f && bossHealth < minimumHealth && bossHealth > 0.05f)
				{
					SetHealth(currentBoss, minimumHealth, 0);
				}
			}
		}
#endif

		for (i = NumEntitiesInTeam("BossLocTeam"); i > 0; i--)
		{
			currentPlayer = GetEntityFromTeam("BossLocTeam", i);
			distanceToBoss = DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr);
			// TODO - FIXME - DGNOTE - this is hardcoded to convert to yards, which be wrong wrong wrong on the French and German servers
			VarSetNumber(EntVar(currentPlayer, "BossDistance"), distanceToBoss * (1.0f / 3.0f));
		}
		for (i = NumEntitiesInTeam("SigilPerm"); i > 0; i--)
		{
			currentPlayer = GetEntityFromTeam("SigilPerm", i);
			distanceToBoss = DistanceBetweenLocations(PointFromEntity(currentPlayer), bossLocationStr);
			// TODO - FIXME - DGNOTE - this is hardcoded to convert to yards, which be wrong wrong wrong on the French and German servers
			VarSetNumber(EntVar(currentPlayer, "BossDistance"), distanceToBoss * (1.0f / 3.0f));
		}
		
		if (TimerElapsed("ApocalypseTimer") || NumEntitiesInTeam("BossTeam") == 0)
		{
			SET_STATE("Epilogue");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("Epilogue") ////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		ON_ENTRY  
			// Hide UI - just hide both since we don't know whicjh one is actually active
			ScriptUIHide("Collection:SigilUI", ALL_PLAYERS);

			// Clean up the timer variables
			for (i = NumEntitiesInTeam(ALL_PLAYERS); i >= 0; i--)
			{
				currentPlayer = GetEntityFromTeam(ALL_PLAYERS, i);
				VarSet(EntVar(currentPlayer, "UITimer"), NULL);
			}

			// reset timers
			DeadlyApocalypseResetTimers();

			SetAIPriorityList("EventTeam", CLEANUP_PL);
			Kill("SigilTeam", false);
			ClearAllWaypoints(ALL_PLAYERS);

			sigilCount = VarGetNumber("SigilCount");
			for (wave = 0; wave < sigilCount; wave++)
			{
				if (sigilMapMarkers[wave] != 0)
				{
					DestroyWaypoint(sigilMapMarkers[wave]);
					sigilMapMarkers[wave] = 0;
				}
			}
			
		END_ENTRY
		//////////////////////////////////////////////////////////////////////////

		// end state timer
		if (DoEvery("StateTimer", STATE_CHANGE_DELAY, NULL))
		{
			SendToAllMapsMessage(StringLocalizeWithReplacements(VarGet("DoneMessage"), 1,
																	"zonename", StringLocalizeMenuMessages(GetMapName())));
			SendPlayerSound(ALL_PLAYERS, VarGet("CleanupSound"), SOUND_MUSIC, 1.0f); 

			Kill("Debt", false); 

			// allow zone spawns
			// SetDisableMapSpawns(false);

			SET_STATE("WaitingToStart");
		}

	//////////////////////////////////////////////////////////////////////////////////
	STATE("EventShutdown") ////////////////////////////////////////////////////////

		DeadlyApocalypseShutdown();

	//////////////////////////////////////////////////////////////////////////////////
	STATE("SpawnAll") ////////////////////////////////////////////////////////

		ON_ENTRY  
			for (i = 0; i < 10; i++)
			{
				snprintf(templateResult, TEMPLATE_RESULT_SIZE, SigilLocationTemplate, i + 1);
				templateResult[TEMPLATE_RESULT_SIZE - 1] = 0;
				currentSigilLocationStr = StringAdd("marker:", templateResult);

				currentSigilName = VarGet("SigilName_1");
				CreateVillain("SigilTeam", currentSigilName, 0, NULL, currentSigilLocationStr);

				sigilMapMarkers[i] = CreateWaypoint(currentSigilLocationStr, "I am a sigil", 
													"map_enticon_code_fullB", NULL, false, false, -1.0f);
				if (sigilMapMarkers[i] > 0)
				{
					SetWaypoint(ALL_PLAYERS, sigilMapMarkers[i]);
				}
			}
		END_ENTRY

	END_STATES

	ON_STOPSIGNAL
		DeadlyApocalypseShutdown();
	END_SIGNAL

	ON_SIGNAL("OneTime")
		SET_STATE("OneTimePause");
	END_SIGNAL

	ON_SIGNAL("Prelude")
		MapSetDataToken(VarGet("DeadlyOneTimeToken"), 1);
		SET_STATE("OneTimePause");
	END_SIGNAL

	ON_SIGNAL("RunMe")
		SET_STATE("RunMe");
	END_SIGNAL

	ON_SIGNAL("Sigils")
		SET_STATE("Sigils");
	END_SIGNAL

	ON_SIGNAL("BossFight")
		SET_STATE("BossFight");
	END_SIGNAL

	ON_SIGNAL("Epilogue")
		SET_STATE("Epilogue");
	END_SIGNAL

	ON_SIGNAL("Restart")
		SET_STATE("WaitingToStart");
	END_SIGNAL

	ON_SIGNAL("SpawnAll")
		SET_STATE("SpawnAll");
	END_SIGNAL
}


void DeadlyApocalypseInit()
{
	SetScriptName("DeadlyApocalypse");
	SetScriptFunc(DeadlyApocalypse);
	SetScriptType(SCRIPT_ZONE);		

	SetScriptVar("MinInvasion",							"1.0",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("MaxInvasion",							"2.0",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);

	SetScriptVar("InvaderLevel",						"30",						SP_OPTIONAL );

	SetScriptVar("SigilCount",							"4",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilPrepareTime",					"4",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilDuration",						"15",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossDuration",						"15",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("MaxDefenders",						"25",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("MaxDefenderWander",					"100",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("DefenderKillsNeeded",					"50",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("DefenderSpawnHeartbeat",				"5",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("UIHijackTime",						"30",						SP_OPTIONAL | SP_DONTDISPLAYDEBUG);

	SetScriptVar("DeadlyEventToken",					"DeadlyEventToken",			SP_OPTIONAL | SP_DONTDISPLAYDEBUG);
	SetScriptVar("DeadlyOneTimeToken",					"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("AlertMessage",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("DoneMessage",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("AlertSound",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("CleanupSound",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("SigilVulnerable",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilDefeated",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilVulnerability",					"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("DestroySigils",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("DestroyBoss",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossDist",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("TitleText",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("TitleHint",							"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("MinionList_1",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("LTList_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossList_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("EBList_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilName_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilMapIcon_1",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilUIIcon_1",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("MinionList_2",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("LTList_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossList_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("EBList_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilName_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilMapIcon_2",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilUIIcon_2",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("MinionList_3",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("LTList_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossList_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("EBList_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilName_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilMapIcon_3",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilUIIcon_3",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("MinionList_4",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("LTList_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("BossList_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("EBList_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilName_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilMapIcon_4",						"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SigilUIIcon_4",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("PowerList_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("PowerList_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("PowerList_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("PowerList_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("PowerActivationCounts",				"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("SpawnFX_1",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SpawnFX_2",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SpawnFX_3",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SpawnFX_4",							"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SpawnFXSpecialCaseCritters",			"",							SP_DONTDISPLAYDEBUG);
	SetScriptVar("SpawnFXSpecialCaseFX",				"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("FinalBossList",						"",							SP_DONTDISPLAYDEBUG);

	SetScriptVar("SigilTranslucentPower",				"",							SP_DONTDISPLAYDEBUG);

	SetScriptSignal("OneTime",		"OneTime");		
	SetScriptSignal("Prelude",		"Prelude");		
	SetScriptSignal("RunMe",		"RunMe");		
	SetScriptSignal("Sigils",		"Sigils");		
	SetScriptSignal("BossFight",	"BossFight");		
	SetScriptSignal("Epilogue",		"Epilogue");		
	SetScriptSignal("Restart",		"Restart");		
	SetScriptSignal("SpawnAll",		"SpawnAll");		
}
