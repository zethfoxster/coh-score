/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UICOSTUME_H__
#define UICOSTUME_H__

#include "uiInclude.h"

#include "Color.h"
#include "costume_data.h"
#include "costume.h"
#include "uiAvatar.h"
#include "AccountTypes.h"

typedef enum CostumeEditMode
{
	COSTUME_EDIT_MODE_STANDARD,
	COSTUME_EDIT_MODE_TAILOR,
	COSTUME_EDIT_MODE_NPC,
	COSTUME_EDIT_MODE_NPC_NO_SKIN,
}CostumeEditMode;

void resetCostumeMenu();
void costumeMenu(void);
void costumeMenuEnterCode(void);
void costumeInit();

void randomCostume(void);
void setDefaultCostume(int useRandomCostumeSetAsDefault);

Color drawColorPalette( F32 x, F32 y, F32 z, F32 scale, const ColorPalette* palette, Color selectedColor, bool anySelected);

typedef struct HybridElement HybridElement;
int drawCostumeRegionButton( const char * text, float x, float y, float z, float wd, float ht, int selected, HybridElement *hb, int flip );

void drawSpinButtons(float screenScaleX, float screenScaleY);
void drawZoomButton(int* isZoomedIn);
void updateCostumeEx(bool resetSets);
#define updateCostume() updateCostumeEx(true)
void costume_drawRegions( CostumeEditMode mode );
void costume_drawColorsScales(CostumeEditMode mode);
void costume_reset(int sgMode);
void costume_SetStartColors(void);
void resetCostume(int do_all);
void tailor_RevertScales( Entity * e );
void clearCostumeStack(void);
int drawAvatarControls(F32 cx, F32 cy, int * zoomedIn, int drawArrowsOnly);

// result of color bar interaction
typedef enum
{
	CB_NONE,
	CB_HIT,
	CB_HIT_ONE,
	CB_HIT_TWO,
} ColorBarResult;

F32 costumeDrawSlider(float x, float y, float scale, int sliderType, char * title,  F32 value, F32 range, float uiScale, float screenScaleX, float screenScaleY );
F32 costumeDrawSliderTriple(float x, float y, float wd, float scale, int sliderType, F32 value, F32 range, float uiscale );
void convergeFaceScale(void);
extern int gCurrentBodyType;
extern int gCurrentRegion;
extern int gCurrentOriginSet;
extern int gTailorColorGroup;
extern int gTailorShowingSGColors;
extern int gTailorShowingCostumeScreen;
extern int gTailorShowingSGScreen;
extern float gTailorSGCostumeScale;

enum
{
	COLOR_1,
	COLOR_2,
	COLOR_3,
	COLOR_4,
	COLOR_SKIN,
	COLOR_TOTAL,
};
enum
{
	REGION_HEAD,
	REGION_UPPER,
	REGION_LOWER,
	REGION_WEAPON,
	REGION_SHIELD,
	REGION_CAPE,
	REGION_GLOWIE,
	REGION_TOTAL_DEF, // total number we acknowledge in the defs
	REGION_SETS = REGION_TOTAL_DEF, // a virtual region
	REGION_TOTAL_UI, // total number of ui regions for local arrays and such
};
enum
{
	REGION_ARACHNOS_EPIC_ARCHETYPE,
	REGION_ARACHNOS_WEAPON,
	REGION_ARACHNOS_AURA,

	REGION_ARACHNOS_TOTAL_UI
};

typedef union Color Color;
typedef struct CostumeGeoSet CostumeGeoSet;
int getIndexFromColor( Color color );
int getSkinIndexFromColor( Color color );

typedef struct CostumeSave CostumeSave;
CostumeSave *costumeBuildSave(CostumeSave * cs);
void costumeClearSave( CostumeSave *cs);
int	costumeApplySave( CostumeSave *cs );

extern int gActiveColor;
extern int gColorsLinked;
extern int gConvergingFace;
extern bool gIgnoreLegacyForSet;
extern float LEFT_COLUMN_SQUISH;

const CostumeGeoSet* getSelectedGeoSet(int mode);
extern int gSelectedCostumeSet;

SkuId isSellableProduct(const char ** costumeKeys, const char * productCode);

typedef struct CostumeBoneSet CostumeBoneSet;
bool bonesetIsLegacy( const CostumeBoneSet *bset );
bool bonesetIsRestricted( const CostumeBoneSet *bset );
bool bonesetIsRestricted_SkipChecksAgainstCurrentRegionSet( const CostumeBoneSet *bset );

typedef struct CostumeTexSet CostumeTexSet;
bool partIsLegacy( const CostumeTexSet *mask );
bool partIsRestricted( const CostumeTexSet *tset );
bool partIsRestricted_SkipChecksAgainstCurrentCostume( const CostumeTexSet *tset );

typedef struct CostumeMaskSet CostumeMaskSet;
bool maskIsLegacy( const CostumeMaskSet *mask );
bool maskIsRestricted( const CostumeMaskSet *mask );

typedef struct CostumeGeoSet CostumeGeoSet;
bool geosetIsLegacy( const CostumeGeoSet *gset );
bool geosetIsRestricted( const CostumeGeoSet *gset );

typedef struct CostumeSetSet CostumeSetSet;
bool costumesetIsRestricted(const CostumeSetSet *set);
bool costumesetIsLocked( const CostumeSetSet *set);

typedef enum CostumeClothPiece
{
	CCP_NONE = 0,
	CCP_TRENCH_COAT = 1,
	CCP_MAGIC_BOLERO = 1 << 1,
	CCP_CAPE = 1 << 2,
	CCP_WEDDING_VEIL = 1 << 3,
	CCP_ALL = ((1 << 4)-1),
}CostumeClothPiece;
bool costume_isOtherRestrictedPieceSelected(const char *currentPart, const char *restriction);
const char *costume_isRestrictedPiece(const char **flags, const char *restriction);

void clearSelectedGeoSet();
char *getCustomCostumeDir();
const char *getCustomCostumeExt();

typedef struct Costume Costume;
typedef struct cCostume	cCostume;
typedef struct CustomNPCCostume CustomNPCCostume;
int costume_isThereSkinEffect(const cCostume *costume);
int costume_setColorTintablePartsBitField(const cCostume *costume);
void costume_NPCCostumeSave(const cCostume *originalCostume, const char *npcName, CustomNPCCostume **NPCCostume);
const CostumeOriginSet*	costume_get_current_origin_set(void);

void costumeMenuExitCode(void);
void updateCostumeWeapons();
int repositionCamera(int doReposition);
void drawZoomButtonHybrid(F32 cx, F32 cy, int* isZoomedIn );
int checkCostumeMatchesBody(BodyType costume, AvatarMode avatarBodyType);

#define ZOOM_Z 5000

#endif /* #ifndef UICOSTUME_H__ */

/* End of File */

