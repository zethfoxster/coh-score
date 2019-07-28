#ifndef UIINSPIRATION_H
#define UIINSPIRATION_H

#include "uiInclude.h"

void fixInspirationTrayToDealWithRealWidthAndHeight(void);

int  inspirationWindow();
void inspiration_activateSlot( int slot_num, int row_num );
void inspiration_findAndActivateSlot( char *pch );
int inspiration_FindByName( char * pchName, int *slot, int *row );
void trayobj_moveInspiration( int iSrcCol, int iSrcRow, int iDestCol, int iDestRow );
void inspirationNewData( int iCol, int iRow );
void inspirationUnqueue();
void inspiration_setMode( int mode );

void inspiration_drag( int i, int j, AtlasTex * icon );

typedef struct Entity Entity;
void updateInspirationTray( Entity * e, int cols, int rows );

typedef struct TrayObj TrayObj;
TrayObj * inspiration_getCur( int i, int j );

typedef struct CBox CBox;
void inspirationWindowFlash(void);
void inspirationMerge( char * pchSourceIn, char * pchDestIn );
void inspirationDeleteByName( char * pchName );

#endif
