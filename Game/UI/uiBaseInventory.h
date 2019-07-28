
#ifndef UIBASEINVENTORY_H
#define UIBASEINVENTORY_H


int baseInventoryWindow(void);

bool baseInventoryIsStyle(void);
void basepropsSetDecorIdx(int idx);

void drawRoomBoxes( const RoomTemplate * room, float x, float y, float z, float wd, float ht );
void drawPlotBox( const BasePlot * plot, float x, float y, float z, float wd, float ht );
void *getBaseInventorySelectedItem();
void setBaseInventorySelectedItem(void *item);


void baseSetBaseCreate();
bool playerIsInBaseEntrance();

typedef enum BaseStatusMsgType
{
	kBaseStatusMsg_Instruction,
	kBaseStatusMsg_PassiveError,
	kBaseStatusMsg_Warning,
	kBaseStatusMsg_Error,
}BaseStatusMsgType;

void base_setStatusMsg( char * str, int type );

extern int g_iDecorIdx;

#endif