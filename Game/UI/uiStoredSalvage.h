/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UISTOREDSALVAGE_H
#define UISTOREDSALVAGE_H

#include "uiInclude.h"

typedef struct SalvageInventoryItem SalvageInventoryItem;
typedef struct uiGrowBig uiGrowBig;
typedef struct ContextMenu ContextMenu;
typedef struct CBox CBox;

void storedSalvageReparse(void);
int storedSalvageWindow(void);
void storedsalvage_drag(int index, AtlasTex *icon, float sc);
void ShowStoredSalvage(bool has_anchor_pos, Vec3 anchor_pos);

void storedsalvage_MoveFromPlayerInv(int amount, char *nameSalvage);
void storedsalvage_MoveToPlayerInv(int amount, char *nameSalvage);

void storedsalvageWindowFlash(void);

#endif //UISTOREDSALVAGE_H
