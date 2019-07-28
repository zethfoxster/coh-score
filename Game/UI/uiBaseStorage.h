/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIBASESTORAGE_H
#define UIBASESTORAGE_H

#include "stdtypes.h"

int basestorageWindow();

typedef struct SalvageInventoryItem SalvageInventoryItem;
typedef struct BasePower BasePower;
typedef struct Boost Boost;
typedef struct RoomDetail RoomDetail;

void salvageAddToStorageContainer(const SalvageInventoryItem *sal, int count);
void inspirationAddToStorageContainer(const BasePower *ppow, int idDetail, int col, int row, int amount);
void enhancementAddToStorageContainer(const Boost *enh, int idDetail, int iInv, int amount);

SalvageInventoryItem * getStoredSalvageFromName( const char * name );

RoomDetail *getStorageWindowDetail(void);
int getShowLogCenter( int wdw, float * xp, float * yp );
void basestorageSetDetail(RoomDetail *detail);

#endif //UIBASESTORAGE_H
