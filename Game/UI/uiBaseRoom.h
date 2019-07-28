
#ifndef UIBASEROOM_H
#define UIBASEROOM_H

typedef struct RoomTemplate RoomTemplate;
typedef struct BaseRoom BaseRoom;
typedef struct BasePlot BasePlot;

int baseRoomWindow(void);

void baseRoomSetStatTab();

char * baseRoomGetDescriptionString( const RoomTemplate *room, BaseRoom * current_room, int showInfo, int showStats, int width);
char * basePlotGetDescriptionString( const BasePlot *plot, int width);
#endif