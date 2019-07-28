/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BASEDATA_H__
#define BASEDATA_H__

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct DetailCategory DetailCategory;
typedef struct Detail Detail;
typedef struct BasePower BasePower;
typedef struct StashTableImp *StashTable;
typedef const struct StashTableImp *cStashTable;

typedef enum AttachType // needs to match s_aAttachTypes
{
	kAttach_Floor,
	kAttach_Wall,
	kAttach_Ceiling,
	kAttach_Surface,	// must be last!

	kAttach_Count,
} AttachType;

typedef enum DetailFunction
{
	kDetailFunction_None  = 0,
	kDetailFunction_Workshop,
	kDetailFunction_Teleporter,
	kDetailFunction_Turret,
	kDetailFunction_Field,
	kDetailFunction_Anchor,
	kDetailFunction_Battery,
	kDetailFunction_Trap,
	kDetailFunction_RaidTeleporter,
	kDetailFunction_ItemOfPower,
	kDetailFunction_ItemOfPowerMount,
	kDetailFunction_Medivac,
	kDetailFunction_Contact,	// Standalone version of _AuxContact
	kDetailFunction_Store,
	kDetailFunction_StorageSalvage,
	kDetailFunction_StorageInspiration,
	kDetailFunction_StorageEnhancement,

	/* Add non-auxiliary functions above here */
	/* Add auxiliary functions below here, change the IS_AUXILIARY macro if needed */

	kDetailFunction_AuxSpot,
	kDetailFunction_AuxApplyPower,
	kDetailFunction_AuxGrantPower,
	kDetailFunction_AuxTeleBeacon,
	kDetailFunction_AuxBoostEnergy,
	kDetailFunction_AuxBoostControl,
	kDetailFunction_AuxMedivacImprover,
	kDetailFunction_AuxWorkshopRepair,
	kDetailFunction_AuxContact,

	kDetailFunction_Count,
} DetailFunction;

#define IS_AUXILIARY(function) ((function)>=kDetailFunction_AuxSpot)
const char *detailfunction_ToStr(DetailFunction e);

//------------------------------------------------------------
//  A detail required for <something> in a base
// - i.e. for placing a trophy
//----------------------------------------------------------
typedef struct DetailRequirement
{
	const Detail *pDetail;
	U32 amount;
} DetailRequirement;

typedef struct DetailBoundsOverride {
	Vec3 vecMin;
	Vec3 vecMax;
} DetailBoundsOverride;

typedef struct Detail
{
	int id;				// for use by GenericInvDictionary
	const char *pchName;
	const char *pchDisplayName;
	const char *pchDisplayHelp;
	const char *pchDisplayShortHelp;
		// Name and help shown to user.

	const char *pchDisplayTabName;
		// The name of the "UI category" (i.e. tab) this detail belongs to.

	const char *pchCategory;
		// The name of the "room category" this detail belongs to

	const char *pchGroupName;
	const char *pchGroupNameMount;
		// GroupDef name for the active (powered) object
	const char *pchGroupNameUnpowered;
	const char *pchGroupNameUnpoweredMount;
		// GroupDef name for the inactive (unpowered) object
	const char *pchGroupNameDestroyed;
		// GroupDef name for the destroyed version of the object
	const char *pchGroupNameBaseEdit;
		// GroupDef name for extra geo that shows up in base editor only

	Mat4 mat;
		// Amount to translate/rotate the GroupDef
	DetailBoundsOverride *pBounds;
		// Optional manual override of bounds
	DetailBoundsOverride *pVolumeBounds;
		// Optional manual override of volume bounds

	float fGridScale;
		// Multiplier of standard grid size
	float fGridMin;
		// Minimum grid setting
	float fGridMax;
		// Maximum grid setting

	int iCostPrestige;
		// How much the detail costs to buy the first time.
	int iCostInfluence;
		// How much the detail costs to buy the first time.

	int iUpkeepPrestige;
		// How much the detail costs each month after the first one.
	int iUpkeepInfluence;
		// How much the detail costs each month after the first one.

	int iEnergyConsume;
	int iEnergyProduce;
		// How much energy the detail produces or consumes.
	int iControlConsume;
	int iControlProduce;
		// How much control the detail produces or consumes.

	int iLifetime;
		// Lifetime of this detail, in seconds (used for batteries).

	float fRepairChance;
		// Base chance of this detail recovering from destruction after a base raid.

	const char *pchEntityDef;
		// EntityDef name to spawn when the map is live.
	int iLevel;
		// The level to spawn the entity at.
	const char *pchBehavior;
		// Behavior string the entity uses for AI

	const char *pchDecaysTo;
		// What detail this will become after it decays
	int iDecayLife;
		// How many seconds this item will have after it days

	const char **ppchAuxAllowed;
		// Names of auxiliary items that can attach to this item
	int iMaxAuxAllowed;
		// The number of aux items which can be attached to this item.

	AttachType eAttach;
		// What it sticks to.

	int iReplacer;
		// this item replaces the floor or ceiling it's connected to.

	bool bObsolete;
		// Cannot be added to a base any longer

	bool bMustBeReachable;
		// This does not have entity, but must not be walled off

	bool bHasVolumeTrigger;
		// Does this detail include a volume used for triggering
	bool bMounted;
		// Does this detail sit on a mount
	bool bAnimatedEnt;
		// Does this detail use an entity with an animated ent type
	bool bIsStorage;
		// Is this detail a storage bin with private permissions

	U32 iFlags[1];
		// See DetailFlag enum

	DetailFunction eFunction;
		// What the item does
	const char **ppchFunctionParams;
		// A list of parameters specific to each function.

	const DetailRequirement **ppRequiredDetails;
		// details needed to place details
	const DetailRequirement **ppDisruptingDetails;
		// details needed to 'disrupt' this detail.

	const char *pchPower;
		// the power that this detail grants to those in the sgroup

	const char **ppchRequires;
		// An expression which describes the conditions under which the
		//   sgroup earns the recipe. Once the sgroup earns a recipe, it is
		//   always known and visible.

	bool bCannotDelete;
		// If true, then the detail cannot be deleted by the architect.

	bool bWarnDelete;
		// If true, warn the detail can only be deleted after confirmation

	bool bDoNotBlock;
		// If true, this detail is ignored when doing blocking calculations

	const char *pchDisplayFloatDestroyed;
	const char *pchDisplayFloatUnpowered;
	const char *pchDisplayFloatUncontrolled;
		// Float text sent to raid defenders when these events occur


	// Not persisted

	bool bSpecial;
		// If true, this detail is also stuck in a list stored in SQL.
		//   It's used for knowing about items of power and other things which
		//   affect the whole supergroup even when the base isn't loaded.

	int uniqueId;
		// used as a handle for associating cached base editor data (extents, contact points)

	int migrateVersion;
	const char *migrateDetail;
		// if this item is placed on a base version under migrateVersion,
		// replace it with migrateDetail in baseFixBaseIfBroken.

} Detail;

typedef enum DetailFlag {
	DetailFlag_Tintable,
		// This detail can be tinted through the UI
	DetailFlag_Logo,
		// This detail needs a texture swap for the logo
	DetailFlag_Autoposition,
		// Automatically add origin point adjustment to center this item for correct
		// placement for the item's default attach type.
	DetailFlag_NoRotate,
		// Rotation is disallowed
	DetailFlag_NoClip,
		// Must be fully inside a room, cannot clip through walls
	DetailFlag_ForceAttach,
		// Override of attach type is disallowed

	DetailFlag_Count
};
STATIC_ASSERT(DetailFlag_Count <= 32);

// TODO: Replace these with the generic flag infrastructure when it is implemented
// in textparser.
#define DetailHasFlag(pDetail, flagName) ( ((pDetail)->iFlags[0] & ( 1 << DetailFlag_##flagName )) != 0 )

typedef struct DetailDict
{
	const Detail **ppDetails;
	cStashTable haItemNames;
	const Detail **itemsById;

	cStashTable tabLists;
} DetailDict;

extern SHARED_MEMORY DetailDict g_DetailDict;


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct DetailCategory
{
	char *pchName;
		// Well-known name

	char *pchDisplayName;
	char *pchDisplayHelp;       // TODO: remove?
	char *pchDisplayShortHelp;  // RODO: remove?
		// Name and help shown to user.

	int iEnergyPriority;
	int iControlPriority;
		// Relative priority settings used to determine which items are
		//   given first chance at the available energy and power.

	Detail **ppDetails;
		// A list of all of the items in the category

} DetailCategory;

typedef struct DetailCategoryDict
{
	const DetailCategory **ppCats;
} DetailCategoryDict;

extern SHARED_MEMORY DetailCategoryDict g_DetailCategoryDict;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct RoomCategory
{
	char *pchName;
	char *pchDisplayName;
	char *pchDisplayHelp;
	char *pchDisplayShortHelp;
		// Name and help shown to user
} RoomCategory;

typedef struct RoomCategoryDict
{
	const RoomCategory **ppRoomCats;
} RoomCategoryDict;

extern SHARED_MEMORY RoomCategoryDict g_RoomCategoryDict;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct DetailCategoryLimit
{
	char *pchCatName;
		// the text name of the detail category

	int iLimit;
		// how many are allowed in the room

	const DetailCategory *pCat;
		// the pointer to the detail category

}DetailCategoryLimit;


typedef struct RoomTemplate
{
	char *pchName;
	char *pchDisplayName;
	char *pchDisplayHelp;
	char *pchDisplayShortHelp;
		// Name and help shown to user.

	int iLength;
	int iWidth;
		// Room size

	int iCostPrestige;
		// Prestige cost of room.

	int iCostInfluence;
		// influence cost of room.

	int iUpkeepPrestige;
		// How much the detail in prestige costs each month after the first one.

	int iUpkeepInfluence;
		// How much the detail costs in influence each month after the first one.

	bool bAllowTeleport;
		// If true, players can teleport into this room during a raid.

	DetailCategoryLimit **ppDetailCatLimits;
		// The items that can be placed and their limits

	char * pchRoomCat;
		// name of the category this room belongs to

	RoomCategory *pRoomCat;
		// the category this room belongs to

	char **ppchRequires;
		// An expression which describes the conditions under which the
		//   room is shown. If empty, the room is shown.

} RoomTemplate;


typedef struct RoomTemplateDict
{
	const RoomTemplate **ppRooms;
} RoomTemplateDict;

extern SHARED_MEMORY RoomTemplateDict g_RoomTemplateDict;


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

typedef struct BasePlot
{
	char *pchName;
	char *pchDisplayName;
	char *pchDisplayHelp;
	char *pchDisplayShortHelp;
		// Name and help shown to user.

	char *pchDisplayTabName;
		// Name of the tab to put the plot in

	char *pchGroupName;
		// Group name of the floor tile piece to be used to build the plot

	int iLength;
	int iWidth;
		// plot size

	int iMaxRaidParty;
		// Maximum size of the group allowed to attack bases built on this
		// size plot.

	int iCostPrestige;
		// prestige cost of plot.
	int iCostInfluence;
		// influence cost of plot.

	int iUpkeepPrestige;
		// How much the plot costs each month after the first one.
	int iUpkeepInfluence;
		// How much the plot costs each month after the first one.

	DetailCategoryLimit **ppDetailCatLimits;
		// The items that can be placed and their limits

	char **ppchRequires;
		// An expression which describes the conditions under which the
		//   plot is shown.

}BasePlot;

typedef struct BasePlotDict
{
	const BasePlot **ppPlots;
}BasePlotDict;

extern SHARED_MEMORY BasePlotDict g_BasePlotDict;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void basedata_Load(void);

const RoomTemplate *basedata_GetRoomTemplateByName(const char *pch);
const BasePlot *basedata_GetBasePlotByName(const char * pch);
const Detail *basedata_GetDetailByName(const char *fullname);
const DetailCategory *basedata_GetDetailCategoryByNameEx(const DetailCategoryDict *pDict, const char *pch);
const DetailCategory *basedata_GetDetailCategoryByName(const char *pchCat);
const Detail ** baseTabList_get(const char * pchName);

const char *detailGetThumbnailName(const Detail *detail);
const char *detail_ReverseLookupFunction(DetailFunction function);

typedef struct Entity Entity;
bool baseDetailMeetsRequires(const Detail * detail, Entity *e);
bool baseRoomMeetsRequires(const RoomTemplate *room, Entity *e);
bool basePlotMeetsRequires(const BasePlot * plot, Entity *e);
#endif /* #ifndef BASEDATA_H__ */

/* End of File */

