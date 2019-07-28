#ifndef GAMETYPES_H
#define GAMETYPES_H

#define MAX_ENTITIES_PRIVATE 10240 // must be multiple of 32 (some wacky networking bitfields)
#define MAX_NAME_LEN 128
#define MAX_ATTRIBNAME_LEN 128
#define MAX_ZOWIE_NAME	128 	// Split this from max_name, 'cause they aren't really the same.
#define MAX_ZOWIE_TYPE	8 		// max number of zowie types for a mission

#define MAPID_ATLAS_PARK			1
#define MAPID_GALAXY_CITY			29
#define MAPID_MERCY_ISLAND			71
#define MAPID_NOVA_PRAETORIA		36

// These are the default colors we use for the UI for each type of player.
// The MapServer changes all the player's windows if they are the old default
// when they switch type or Praetorian-ness.
#define DEFAULT_HERO_WINDOW_COLOR		0x3399ffff
#define DEFAULT_VILLAIN_WINDOW_COLOR	0xa62929ff
#define DEFAULT_PRAETORIAN_WINDOW_COLOR	0x707072ff

typedef enum MapTeamArea
{
	MAP_TEAM_HEROES_ONLY,			// Primal heroes only
	MAP_TEAM_VILLAINS_ONLY,			// Primal villains only
	MAP_TEAM_PRIMAL_COMMON,			// Heroes & villains but no Praetorians
	MAP_TEAM_PRAETORIANS,			// Only Praetorians, no primal characters
	MAP_TEAM_EVERYBODY,				// No restrictions whatsoever

	MAP_TEAM_COUNT					// Used in STATIC_ASSERT() and for errors
} MapTeamArea;

// I hate this soooo much
#define MAP_HERO_START				1
#define MAP_MIDNIGHTER_CLUB			4
#define MAP_OUROBOROS				16
#define MAP_VILLAIN_START			71
#define MAP_PRAETORIAN_START		36
#define MAP_POCKET_D				83

// These are where Praetorians arrive when they travel to Primal Earth.
#define MAP_PRAETORIAN_ARRIVE_HERO		8
#define MAP_PRAETORIAN_ARRIVE_VILLAIN	74

typedef enum
{
	GENDER_UNDEFINED = 0,
	GENDER_NEUTER,
	GENDER_MALE,
	GENDER_FEMALE
} Gender;

typedef enum PlayerType
{
	kPlayerType_Hero,
	kPlayerType_Villain,
	kPlayerType_Count
	// If you add a new type look in the code for uses of playerType.
	// It is sent to/from the client as a single bit.
	// also sent to/from raids as a single bit
	//
	// If you decide to touch this (I decided otherwise), I strongly recommend
	// you put STATIC_ASSERTS on kPlayerType_Count in any other code you touch.
} PlayerType;

typedef enum PlayerSubType
{
	// it'd be nice to have these be in a better order, but 'normal' needs to be the default
	kPlayerSubType_Normal,
	kPlayerSubType_Paragon,	// more extreme to one side
	kPlayerSubType_Rogue,	// switching sides
	kPlayerSubType_Count
} PlayerSubType;

typedef enum PraetorianProgress
{
	kPraetorianProgress_PrimalBorn,		// not praetorian at all, this is the default
	kPraetorianProgress_Tutorial,		// still in the tutorial
	kPraetorianProgress_Praetoria,		// finished tutorial, locked in praetoria
	kPraetorianProgress_PrimalEarth,	// come to primal earth, can't get back
	kPraetorianProgress_TravelHero,		// TEMPORARY STATE - en route to Primal Earth (Paragon City)
	kPraetorianProgress_TravelVillain,	// TEMPORARY STATE - en route to Primal Earth (Rogue Isles)
	kPraetorianProgress_NeutralInPrimalTutorial,	// primal-born character that hasn't picked a side yet
	kPraetorianProgress_Count
} PraetorianProgress;

// NOTE: DO NOT CHANGE ORDER OF THESE BITS
typedef enum DbFlag
{
	DBFLAG_TELEPORT_XFER		= 1 << 0,
	DBFLAG_UNTARGETABLE			= 1 << 1,
	DBFLAG_INVINCIBLE			= 1 << 2,
	DBFLAG_DOOR_XFER			= 1 << 3,
	DBFLAG_MISSION_ENTER		= 1 << 4,
	DBFLAG_MISSION_EXIT			= 1 << 5,
	DBFLAG_INVISIBLE			= 1 << 6,
	DBFLAG_INTRO_TELEPORT		= 1 << 7,
	DBFLAG_MAPMOVE				= 1 << 8,
	DBFLAG_CLEARATTRIBS		 	= 1 << 9,
	DBFLAG_NOCOLL				= 1 << 10,
	DBFLAG_BASE_ENTER			= 1 << 11,
	DBFLAG_RENAMEABLE			= 1 << 12,
	DBFLAG_BASE_EXIT			= 1 << 13,
	DBFLAG_ALT_CONTACT			= 1 << 14,
	DBFLAG_ARCHITECT_EXIT		= 1 << 15,
	DBFLAG_PRAET_SG_JOIN		= 1 << 16,
	DBFLAG_HALF_MAX_HEALTH		= 1 << 17,
	DBFLAG_UNLOCK_HERO_EPICS	= 1 << 18,
	DBFLAG_UNLOCK_VILLAIN_EPICS	= 1 << 19,
	// Only add new bits here
} DbFlag;

typedef enum AccessLevel {
	ACCESS_USER = 0,		// regular users
	ACCESS_VENDORS = 1,		// marketing and third party vendor approved commands
	ACCESS_COMMUNITY = 2,	// community approved commands to inspect other player's data
	ACCESS_GM_LOW = 3,		// lowest tier game master approved commands
	ACCESS_GM_MEDIUM = 4,	// medium tier game master approved commands
	ACCESS_GM_HIGH = 5,		// highest tier game master approved commands
	ACCESS_ADMIN = 6,		// game master commands that can corrupt player data and should be used only in rare cases
	ACCESS_NOC = 7,			// commands that the NOC can use to adjust server configuration on the fly
	ACCESS_LIVE_DEBUG = 8,	// developer commands that may be used on production servers
	ACCESS_DEBUG = 9,		// developer commands that should not be used on production servers
	ACCESS_CRASH = 10,		// commands that will almost certainly crash the servers
	ACCESS_INTERNAL = 11,	// for internal cross server commands only
} AccessLevel;

typedef enum SlotLock
{
	SLOT_LOCK_NONE,
	SLOT_LOCK_VIP_FREE,
	SLOT_LOCK_OFFLINE,
} SlotLock;

#endif
