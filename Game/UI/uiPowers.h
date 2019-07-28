
#ifndef UIPOWERS_H
#define UIPOWERS_H

// This function displays the primary and secondary power menus
//
void levelPowerPoolEpicMenu(void);

// 0 - primary, 1 - secondary, 2 - power pool, 3 - epic
extern int gCurrentPowerSet[4]; // the primary and secondary power set (index into available powersets on character)
extern int gCurrentPower[4];	// the primart and secondary powers ( index into availble powers on character )

typedef struct Character Character;
typedef struct BasePower BasePower;
//extern BasePower * g_pBasePowMouseover = 0;


int powerSetSelector(int y, int category, Character *pchar, int *iPset, int *iPow, float screenScaleX, float screenScaleY, int *offset, int menu);
int powerSelector( int category, int y, Character * pchar, int *pSet, int *pPow, float screenScaleX, float screenScaleY, int *offset );
void displayPowerHelpBackground(float screenScaleX, float screenScaleY);
void drawMenuPowerFrame( float lx,  float ly,  float lwd, float lht, float swd, float sht,
						float rwd, float rht, float frontZ, float backZ, float screenScaleX,
						float screenScaleY, int color, int back_color );

void power_reset(void);
void powerSetCategory(int iCat);
void powersMenu_resetSelectedPowers();
#endif
