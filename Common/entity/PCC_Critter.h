#ifndef PCC_Critter_H__
#define PCC_Critter_H__

#include "textparser.h"
#include "costume.h"

typedef struct Entity Entity;
typedef struct PCC_Critter
{
	char *name;					//	boss name
	char *description;			//	boss description
	char *villainGroup;			//	boss villain group
	char *primaryPower;			//	boss primary powerSet
	char *secondaryPower;		//	boss secondary powerSet
	const char *travelPower;			//	boss travel powerSet
	int rankIdx;
	int difficulty[2];				//	boss difficulty
	int selectedPowers[2];
	Costume *pccCostume;		//	pointer to costume
	CostumeDiff *compressedCostume;	//	compressedCostume	
	int isRanged;				//	bit for fighting preference (range or melee)
	char *legacyRankText;
	char *referenceFile;		//	reference file (so that people's file path's aren't exposed when these things are passed around)
	char *sourceFile;			//	source file
	int *badCostumeParts;				//	for client only to display bad parts of costumes
	int timeAge;
}PCC_Critter;



enum PCC_LOADERROR_CODES
{
	PCC_LE_VALID = 0,
	PCC_LE_NULL = 1,
	PCC_LE_NO_FILE = 1<<1,
	PCC_LE_BAD_PARSE = 1<<2,
	PCC_LE_MUST_EXIT = 1<<3,
	PCC_LE_INVALID_NAME = 1<<4,
	PCC_LE_INVALID_VILLAIN_GROUP = 1<<5,
	PCC_LE_CORRECTABLE_ERRORS = 1<<6,
	PCC_LE_INVALID_RANGED = 1<<7,
	PCC_LE_INVALID_PRIMARY_POWER = 1<<8,
	PCC_LE_INVALID_SECONDARY_POWER = 1<<9,
	PCC_LE_INVALID_TRAVEL_POWER = 1<<10,
	PCC_LE_BAD_COSTUME = 1<<11,
	PCC_LE_INACCESSIBLE_COSTUME = 1<<12,
	PCC_LE_INVALID_RANK = 1<<13,
	PCC_LE_FAILED_TO_REMOVE = 1<<14,
	PCC_LE_CONFLICTING_POWERSETS = 1 << 15,
	PCC_LE_PROFANITY_NAME = 1 << 16,
	PCC_LE_PROFANITY_VG = 1 << 17,
	PCC_LE_PROFANITY_DESC = 1 << 18,
	PCC_LE_INVALID_DIFFICULTY = 1 << 19,
	PCC_LE_INVALID_DIFFICULTY2 = 1 << 20,
	PCC_LE_INVALID_DESC = 1 << 21,
	PCC_LE_INVALID_SELECTED_PRIMARY_POWERS = 1 << 22,
	PCC_LE_INVALID_SELECTED_SECONDARY_POWERS = 1 << 23,
};
typedef enum PCC_Rank
{
	PCC_RANK_MINION,
	PCC_RANK_LT,
	PCC_RANK_BOSS,
	PCC_RANK_ELITE_BOSS,
	PCC_RANK_ARCH_VILLAIN,
	PCC_RANK_CONTACT,
	PCC_RANK_COUNT,
}PCC_Rank;

typedef enum PCC_Difficulty
{
	PCC_DIFF_STANDARD,
	PCC_DIFF_HARD,
	PCC_DIFF_EXTREME,
	PCC_DIFF_CUSTOM = -1,
	PCC_DIFF_COUNT = 4,
};
// Header for Player Created Critter Editor
typedef enum pcc_MenuCategory
{
	pcc_MenuPrimary,
	pcc_MenuSecondary,
	pcc_MenuTravel,
	pcc_MenuCount,
}pcc_MenuCategory;

#define MAX_CRITTER_POWERS 50 // no idea what this should be

typedef struct PCC_CritterRewardMods
{
	const F32 *fDifficulty;
	const F32 *fPowerNum;
	const F32 *fMissingRankPenalty;
	const F32 fAmbushScaling;
}PCC_CritterRewardMods;

extern SHARED_MEMORY PCC_CritterRewardMods g_PCCRewardMods;

PCC_Critter * clone_PCC_Critter(PCC_Critter *source);
void destroy_PCC_Critter(PCC_Critter *pcc_Critter);
int isPCCCritterValid(Entity *e, PCC_Critter *pcc_Critter, int fixup, int *playableErrorCode);
int getPCCPowerIndexFromPowersetName(const char *powerSetName, int category);
const char *getPCCPowerDisplayNameFromPowersetName( const char *powerSetName, int category);
void generatePCCErrorText_ex(int retVal, char **errorText);
extern TokenizerParseInfo parse_PCC_Critter[];
extern StaticDefineInt PCCRankEnum[];
extern StaticDefineInt PCCDiffEnum[];

extern const PowerCategory *missionMakerPowerSet[3];
#endif