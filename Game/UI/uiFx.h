#ifndef UIFX_H
#define UIFX_H

#include "stdtypes.h"
#include "uiInclude.h"

void electric_initEx( float x, float y, float z, float scale, int light, int duration, int destwdw, int id, int *offset);
int electric_init( float x, float y, float z, float scale, int light, int duration, int count, int *offset );
void electric_removeById( int id );
void electric_clearAll();

typedef struct TTDrawContext TTDrawContext;

void fadingTextEx( float x, float y, float z, float scale, int color1, int color2, int flags, TTDrawContext *font, int duration, int destwdw, char * txt, float expand_rate );
void fadingText( float x, float y, float z, float scale, int color1, int color2, int duration, char * txt );
void fadingText_clearAll();

void attentionText_add(char *pch, U32 color, int wdw, char *pchSound);
void attentionText_clearAll(void);

void priorityAlertText_add(char *pch, U32 color, char *pchSound);
void priorityAlertText_clearAll(void);

void movingIcon_add( AtlasTex *icon, float fromX, float fromY, float fromSc, float toX, float toY, float toSc, float z );
void movingIcon_clearAll(void);

void fx_doAll();

extern int gElectricAction;

#endif