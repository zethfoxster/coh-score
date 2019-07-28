#ifndef UISPUERCOSTUME_H
#define UISPUERCOSTUME_H

void supercostumeMenu();
void supercostumeTailorMenu(float scaleOverride, int sgMode, float screenScaleX, float screenScaleY);
void supercostume_SetEmblem( char *name );

typedef struct Entity Entity;
typedef struct CostumePart CostumePart;
typedef struct CostumeOriginSet CostumeOriginSet;
typedef struct Costume Costume;

void load_CostumeWeaponStancePairList();

void costume_MatchPart( Entity *e, const CostumePart *cpart, const CostumeOriginSet *cset, int supergroup, const char * partName );
void costume_setCurrentCostume( Entity *e, int supergroup );
void costume_SetStanceFor( const CostumePart *cpart, int *stanceBits);
void costume_ZeroStruct( CostumeOriginSet *cset );

extern Costume *gSuperCostume;

#endif