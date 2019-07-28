
#ifndef UIPOWERINFO_H
#define UIPOWERINFO_H

typedef struct BasePower BasePower;
typedef struct Power Power;
typedef struct CharacterClass CharacterClass;
typedef struct ToolTip ToolTip;
typedef struct CBox CBox;
typedef struct StuffBuff StuffBuff;
typedef struct VillainDef VillainDef;
typedef struct AttribModTemplate AttribModTemplate;

extern const BasePower * g_pBasePowMouseover;
extern const Power *g_pPowMouseover;

#define POWERINFO_BASE 0
#define POWERINFO_ENHANCED 1
#define POWERINFO_BOTH 2
#define POWERINFO_HYBRID_BASE 3

void powerInfoSetLevel(int iLevel); 
void powerInfoSetClass(const CharacterClass *pClass); 

void powerInfoClassSelector();
void markPowerOpen(int idx);
void markPowerClosed(int idx);

void displayPowerInfo(F32 x, F32 y, F32 z, F32 wd, F32 ht, const BasePower *ppow, const Power * pow, int window, int ShowEnhance, int isPlayerCreatedCritter );
void menuDisplayPowerInfo( F32 x, F32 y, F32 z, F32 wd, F32 ht, const BasePower *pBasePow, const Power * pPow, int ShowEnhance);
void menuHybridDisplayPowerInfo( F32 x, F32 y, F32 z, F32 wd, F32 ht, const BasePower *pBasePow, const Power * pPow, int ShowEnhance, F32 alphaScale);
int power_addEnhanceStats( const BasePower *ppow, F32 baseVal, F32 combineVal, F32 replaceVal, StuffBuff * str, int noPlus, int offset, const CharacterClass * petClass, const CharacterClass * creatorClass );

bool pet_IsAutoOnly(const AttribModTemplate *ptemplate);

#endif