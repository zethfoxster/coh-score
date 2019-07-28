/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef TRAYCOMMON_H__
#define TRAYCOMMON_H__

#include "character_inventory.h"

#define TRAY_WD					( 10 )
#define TRAY_HT					( 1 )
#define TRAY_SLOTS				( TRAY_WD * TRAY_HT )

typedef struct BasePower BasePower;

//------------------------------------------------------------
// enum type for backwards compatability. old code
// refers to current_tray, now uses current_trays[kCurTrayType_Primary]
// current_alt_tray to current_trays[kCurTrayType_Alt]
//
// -AB: created :2005 Jan 26 03:28 PM
//----------------------------------------------------------
typedef enum
{
    kCurTrayType_Primary,
    kCurTrayType_Alt,
    kCurTrayType_Alt2,
	kCurTrayType_Razer,
    // NOTE: you must update containerloadsave.c if you add elems here
    
    // enum counter
    kCurTrayType_Count
} CurTrayType;

#define NUM_TRAY_WINDOWS 8

#define NUM_CURTRAYTYPE_BITS (2)

//------------------------------------------------------------
// helper to classify the current momentary alt state.
// the momentary state overrides the sticky state if the sticky
// state is less than the alt lvl
//
// the bits here must correspond with the CurTrayType enum above
//  i.e. if t is a CurTrayType enum, than its corresponding alt bit is
//   1<<t
// -AB: created :2005 Jan 31 11:45 AM
//----------------------------------------------------------
typedef enum
{
    kCurTrayMask_NONE,
    kCurTrayMask_Primary = 1,
    kCurTrayMask_ALT = kCurTrayMask_Primary<<1,
    kCurTrayMask_ALT2 = kCurTrayMask_ALT<<1,
    kCurTrayMask_ALL = (kCurTrayMask_ALT2 | kCurTrayMask_ALT | kCurTrayMask_Primary | kCurTrayMask_NONE),
} CurTrayMask;

CurTrayMask curtraytype_to_altmask( CurTrayType var );

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct TexBind TexBind;
typedef struct Boost Boost;
typedef struct uiTab uiTab;
typedef struct AtlasTex AtlasTex;
typedef struct SalvageInventoryItem SalvageInventoryItem;

// New TrayObj Code
//-------------------------------------------------------------------------------------------------

typedef enum TrayItemType
{
	kTrayItemType_None,
	kTrayItemType_Power,
	kTrayItemType_Inspiration,
	kTrayItemType_BodyItem,
	kTrayItemType_SpecializationPower,
	kTrayItemType_SpecializationInventory,
	kTrayItemType_Macro,
	kTrayItemType_RespecPile,
	kTrayItemType_Tab,
	kTrayItemType_ConceptInvItem,
	kTrayItemType_PetCommand,
	kTrayItemType_Salvage,
	kTrayItemType_Recipe,
	kTrayItemType_StoredInspiration,
	kTrayItemType_StoredEnhancement,
	kTrayItemType_StoredSalvage,
	kTrayItemType_StoredRecipe,
	kTrayItemType_MacroHideName,
	kTrayItemType_PersonalStorageSalvage,
	kTrayItemType_PlayerSlot,
	kTrayItemType_PlayerCreatedMission,
	kTrayItemType_PlayerCreatedDetail,
	kTrayItemType_GroupMember,
// 	kTrayItemType_BaseDetail,

	// --------------------
	kTrayItemType_Count,

} TrayItemType;
InventoryType InventoryType_FromTrayItemType(TrayItemType type); 


enum
{
	TO_NONE,
	TO_SCALE_DOWN,
};

#define IS_TRAY_MACRO(type) (type==kTrayItemType_Macro||type==kTrayItemType_MacroHideName)

typedef struct TrayObj TrayObj;
typedef void (*trayobj_display)(TrayObj *obj, F32 x, F32 y, F32 z, F32 sc);

typedef enum TrayCategory
{
	kTrayCategory_Unknown,
	kTrayCategory_PlayerControlled,
	kTrayCategory_ServerControlled,
	kTrayCategory_Count
} TrayCategory;

typedef struct TrayObj
{
	TrayItemType    type;
	AtlasTex*		icon;

	int             state;
	float           scale;
	float			fall_offset;
	float           angle;
	float			flash_scale;
	F32				z;

	// ARM NOTE: these used to be only for the copy operation
	// I've tried to alter them so they are always correct for items in the tray
	// TrayObjs are used in some non-tray applications... which makes it a bit weird.
	TrayCategory	icat;
	int				itray;
	int				islot;

	// Only for powers and inspirations
	int             ipow;

	// Only for powers
	int             iset;
	int				autoPower;
	const char      *pchPowerName;
	const char      *pchPowerSetName;
	const char      *pchCategory;

	// for macros
	char			command[256];
	char			shortName[32];
	char			iconName[256];

	// only for specializations
	int				ispec;

	// for respecialization (where indices become meaningless)
	Boost *pBoost;

	// for Tabs
	uiTab*			tab;

	// for stored items
	union
	{
		const char *nameSalvage;
		const char *nameRecipe;
		const BasePower *ppow;
		Boost *enhancement;
	};
	int idDetail;
	
	// for random inventory items

	trayobj_display displayCb;
	int invIdx;	

} TrayObj;

typedef struct TrayInternals TrayInternals;

// It would be nice if, over time, all variables in Tray migrated into TrayInternals
// and we could get rid of all the public members.  Yay modularity!
typedef struct Tray
{
	int	current_trays[kCurTrayType_Count];
	int	mode;					// used to serialize whether the alt trays are open to the database
	int	mode_alt2;				// "
	int	trayIndexes;			// contains 8 four-bit numbers that are the indices of the eight possible trays (1-9)
	TrayInternals	*internals;	// private data that shouldn't be exposed to the external world.
} Tray;

typedef struct Packet Packet;
typedef struct Entity Entity;

void buildPowerTrayObj(TrayObj *pto, int iset, int ipow, int autoPower );
void buildInspirationTrayObj(TrayObj *pto, int iCol, int iRow);
void buildSpecializationTrayObj(TrayObj *pto, int inventory, int iSet, int iPow, int iSpec );
void buildMacroTrayObj( TrayObj *pto, const char * command, const char *shortName, const char *iconName, int hideName );
void builPlayerSlotTrayObj( TrayObj *pto, int src_idx, char * name );

void sendTray( Packet *pak, Entity *e );
void receiveTray( Packet *pak, Entity *e );

// Create a TrayObj from a memorypool
TrayObj* trayobj_Create(void);
// Destroy a TrayObj created from the memorypool
void trayobj_Destroy( TrayObj *item );

Tray *tray_Create(void);
void tray_Destroy(Tray *tray);

// Returns the trayobj corresponding to a certain location,
// creating it if necessary and flagged, or NULL if it doesnt exist
TrayObj* getTrayObj(Tray *tray, TrayCategory category, int trayIndex, int slotIndex, int buildIndex, bool createIfNotFound);
// Returns the type of the specified location in a tray, or none if that object doesnt exist
TrayItemType typeTrayObj(Tray *tray, TrayCategory category, int trayIndex, int slotIndex, int buildIndex);
// Destroy the trayobj corresponding to a certain location
void clearTrayObj(Tray *tray, TrayCategory category, int trayIndex, int slotIndex, int buildIndex);


// Use these tray iterator functions when you want to iterate over every item in all trays
//   - INCLUDING SERVER CONTROLLED TRAYS!
// Don't expose the guts of the player/server controlled trays to external users!

// Pass -1 to onlyThisBuild to iterate through all builds!
// Pass -1 to onlyThisCategory to iterate through all categories!
void resetTrayIterator(Tray *tray, int onlyThisBuild, TrayCategory onlyThisCategory);
bool getNextTrayObjFromIterator(TrayObj **obj);
void createCurrentTrayObjViaIterator(TrayObj **obj);
void destroyCurrentTrayObjViaIterator();
void getCurrentTrayIteratorIndices(TrayCategory *category, int *tray, int *slot, int *build);

void trimExcessTrayObjs(Tray *tray, int buildCount);

typedef struct TrayItemIdentifier // only for boosts, inspirations, recipes and salvage
{
	char *name;
	int buyPrice; // for things you can buy from the auction house that players don't put up for sale
	TrayItemType type;
	int level;
	int playerType; // 0 for all, 1 for heroes only, 2 for villains only
} TrayItemIdentifier;

char* TrayItemIdentifier_Str(TrayItemIdentifier *ptii);
TrayItemIdentifier* TrayItemIdentifier_Struct(const char *str);

const char* TrayItemType_Str(TrayItemType type);

void tray_verifyNoDoubles(Tray *tray, int build);

int tray_changeIndex(TrayCategory category, int previousValue, int forward);

#endif /* #ifndef TRAYCOMMON_H__ */

/* End of File */

