#ifndef UIINFO_H
#define UIINFO_H

#include "uiInclude.h"

typedef struct BasePower BasePower;
typedef struct Power Power;
typedef struct Detail Detail;
typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct SalvageInventoryItem SalvageInventoryItem;
typedef struct RoomTemplate RoomTemplate;
typedef struct SalvageItem SalvageItem;
typedef struct BasePlot BasePlot;
typedef struct DetailRecipe DetailRecipe;

void info_Power( const BasePower *pow, int forTarget);
void info_SpecificPower( int setIdx, int powIdx);
void info_Entity( Entity * e, int tab);
void info_EntityTarget( void *unused );
void info_Item( const Detail *item, AtlasTex *pic );
void info_Room( const RoomTemplate *room );
void info_Plot( const BasePlot * plot );
void info_replaceTab( const char * name, const char * text, int selected );
void info_addTab( const char * name, const char * text, int selected );
void info_setShortInfo(const char *shortInfo);
void info_addBadges( Packet * pak );
void info_addSalvageDrops(Packet *pak);
void info_clear();
void info_noResponse(void);
void info_Salvage(const SalvageItem *inv );
void info_SalvageInventory( SalvageInventoryItem *inv );
void info_Recipes( const DetailRecipe *pRec );
int  infoWindow();

typedef enum EPowInfo {
	kPowInfo_Normal,
	kPowInfo_Skill,
	kPowInfo_Combine,
	kPowInfo_Respec,
	kPowInfo_SpecLevel,
}
EPowInfo;


void info_PowManagement( EPowInfo mode, float screenScaleX, float screenScaleY );

void info_combineMenu( void * pow, int isSpec, int inventory, int level, int numCombines );
void info_CombineSpec( const BasePower *pow, int inventory, int level, int numCombines );

#endif
