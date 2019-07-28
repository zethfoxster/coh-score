#ifndef UILEVELPOWER_H
#define UILEVELPOWER_H

#include "uiInclude.h"

typedef struct PowerSet PowerSet;
typedef struct BasePowerSet BasePowerSet;
typedef struct BasePower BasePower;
typedef struct Character Character;

typedef struct NewPower
{
	PowerSet		*pSet;
	const BasePowerSet	*baseSet;
	const BasePower	*basePow;
} NewPower;

#define FRAME_OFFSET 40

void levelPowerMenu();
void levelPowerMenu_ForceInit();
int powerLevelShowNew( NewPower *np, Character *pchar, int x, int y, int category, int *pSet, int *pPow, float screenScaleX, float screenScaleY );
int powerLevelSelector( NewPower *np, Character *pchar, int specLevel, int x, int y, int category, int *pSet, int *pPow, bool forceUnavailable, float screenScaleX, float screenScaleY );
void powerSetHelp( AtlasTex * icon, const char * txt );
void displayPowerLevelHelp(float screenScaleX, float screenScaleY);

#endif