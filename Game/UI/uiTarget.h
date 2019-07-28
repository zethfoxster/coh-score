#ifndef TARGET_H
#define TARGET_H

#include "uiInclude.h"

int targetWindow( void );
int targetOptionsWindow( void );

extern int  gSelectedDBID;
extern char gSelectedName[64];
extern char gSelectedHandle[64];

typedef struct Entity Entity;
void contextMenuForTarget( Entity *e );
void contextMenuForOffMap( void );
int getConningColor( Entity *e, Entity *target );
void drawConningArrows(Entity *e, Entity *eTarget, float x, float y, float z, float scale, int arrow_color);

//-------------------------------------------------------------------------------------------------
// Public target interface
//-------------------------------------------------------------------------------------------------
int targetSelect(Entity* e);
int targetSelect2(int dbid, char* name);
void targetSetHandle(char * handle);
int targetSelectHandle(char * handle, int db_id);
int targetHasSelection(void);
void targetClear(void);
int targetGetDBID(void);
char* targetGetName(void);
char* targetGetHandle(void);
bool OfflineTarget();
void targetAssist(char * name);
void setCurrentTargetByName(char * name);

int isCurrentTarget();
#endif