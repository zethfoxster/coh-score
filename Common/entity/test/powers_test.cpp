#if 1

// disabling unittest for now
#include "UnitTest++.h"

#else

#include "powers.c"
#include "UnitTest++.h"

static void * StructLinkFromString(struct StructLink *,char const *) {return 0;}
static char const * allocAddString(char const *) {return 0;}
static char * StructLinkToString(struct StructLink *,const void *,char *,unsigned int) {return 0;}
static struct Power * character_OwnsPower(struct Character *,const struct BasePower *) {return 0;}
static void aiAddPower(struct Entity *,struct Power *) {}
static void aiRemovePower(struct Entity *,struct Power *) {}
static int CountForLevel(int,const int *) {return 0;}
static int character_GetLevel(struct Character *) {return 0;}
static SHARED_MEMORY struct Schedules g_Schedules = {0};
static void MiningAccumulatorAdd(char const *,char const *,int) {}
static void boost_Destroy(struct Boost *) {}
static struct Boost * boost_Create(const struct BasePower *,int,struct Power *,int,int,float const *,int,struct PowerupSlot const *,int) {return 0;}
static void attribmod_Destroy(struct AttribMod *) {}
static void character_SendCombineResponse(struct Entity *,int,int) {}
static void character_SendBoosterResponse(struct Entity *e, int iSuccess, int iSlot ) {}
static void character_SendCatalystResponse(struct Entity *e, int iSuccess, int iSlot ) {}
static struct PowerSpec * powerSpec_CreateFromPower(struct Power *) {return 0;}
static void dbLog(char const *,struct Entity *,char const *,...) {}
static char * dbg_BasePowerStr(const struct BasePower *) {return 0;}
static struct Boost * character_RemoveBoost(struct Character *,int,char const *) {return 0;}
static void character_DestroyBoost(struct Character *,int,char const *) {}
static float boost_CombineChance(const struct Boost *,const struct Boost *) {return 0;}
static float boost_BoosterChance(const struct Boost *pbA) {return 0.0f;}
static void listFreeMember(void *,void * *) {}
static void * listAddNewMember_dbg(void * *,int,char *,int) {return 0;}
static void * listAddNewMember_dbg(void * *,int) {return 0;}
static void listClearList(void * *) {}
static int character_CountPoolPowerSetsBought(struct Character *) {return 0;}
static int character_CountBoostsBought(struct Character *) {return 0;}
static int character_CountPowersBought(struct Character *) {return 0;}
static int character_AdjustSalvage(struct Character *p, struct SalvageItem const *pSalvageItem, int amount, const char *context, int bNoInvWarning ) {return 0;}
static bool character_CanAdjustSalvage(struct Character *p, struct SalvageItem const *pSalvageItem, int amount ) {return false;}
static bool power_IsValidBoost(const struct Power *,const struct Boost *) {return 0;}
static bool character_IsAllowedToHavePowerHypothetically(struct Character *,const struct BasePower *) {return 0;}
static EntityInfo entinfo[1];
static int chareval_Eval(struct Character *,const char * const *) {return 0;}
static bool ParserLoadFiles(const char *,const char *,const char *,int,struct ParseTable * const,void *,struct DefineContext *,struct ParserLoadInfo *,ParserLoadPreProcessFunc) {return 0;}
static void chareval_AddDefaultFuncs(struct EvalContext *) {}
static float combateval_Eval(struct Entity *,struct Entity *,const char **) {return 0.f;}
static struct EvalContext * eval_Create(void) {return 0;}
static struct Entity * erGetEnt(__int64) {return 0;}
static struct ColorPair colorPairNone;
static bool isNullOrNone(char const *) {return 0;}
static struct PowerCustomization * powerCust_FindPowerCustomization(struct PowerCustomizationList *,int) {return 0;}
static struct PowerCustomizationList * powerCustList_current(struct Entity *) {return 0;}
static struct AtlasTex * atlasLoadTexture(const char *) {return 0;}
static struct AtlasTex * white_tex_atlas;
static const struct SalvageItem* salvage_GetItem( char const *name ) {return 0;}
static SHARED_MEMORY struct BoostSetDictionary g_BoostSetDict = {0,0};

TEST(craftedPowersetTest) {
	BasePowerSet set = {0};
	CHECK(!baseset_IsCrafted(NULL));
	set.pchName = "Crafted_Luck_of_the_Gambler_A";
	CHECK(baseset_IsCrafted(&set));
}
#endif
