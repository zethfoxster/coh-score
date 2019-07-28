#ifndef UITRADE_H
#define UITRADE_H

#include "uiTradeLogic.h"

int tradeWindow();

typedef struct BasePower BasePower;
typedef struct Power Power; 
void trade_setInspiration( const BasePower * pow, int i, int j );
void salvMarkForTrade( Entity *e,int idx, int count );
void salvRemoveFromTrade(int idx, int count);
void specMarkForTrade( Entity *e, int i );
void inspMarkForTrade( Entity *e, int i, int j );
bool powerIsMarkedForTrade( Power * pow );
void uiTrade_UpdateRecipeInventoryCount( Entity *e, int idx, int id );
void uiTrade_UpdateSalvageInventoryCount( Entity *e,int idx );
int inspIsMarkedForTrade( Entity *e, int i, int j );

#endif