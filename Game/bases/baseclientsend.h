#ifndef _BASECLIENTSEND_H
#define _BASECLIENTSEND_H

typedef struct RoomDetail RoomDetail;
typedef struct BaseRoom BaseRoom;
typedef struct RoomTemplate RoomTemplate;
typedef struct BasePlot BasePlot;
typedef struct RoomDoor RoomDoor;

int baseWaitingForServer();

void baseClientSendHeight(BaseRoom *room,int type,int idx,F32 floor,F32 ceiling);
void baseClientSendTexSwap(BaseRoom *room,int partIdx,char *texname);
void baseClientSendGeoSwap(BaseRoom *room,int partIdx,char *style);
void baseClientSendTint(BaseRoom *room,int partIdx,Color colors[2]);
void baseClientSendLight(BaseRoom *room,int idx,Color light_color);

void baseClientSendDoorway(BaseRoom *room,int block[2],F32 height[2]);
void baseClientSendDoorwayDelete(BaseRoom *room,int block[2]);

void baseClientSendDetailCreate(BaseRoom *room,RoomDetail *detail);
void baseClientSendDetailUpdate(BaseRoom *room,RoomDetail *detail,BaseRoom *newroom);
void baseClientSendDetailDelete(BaseRoom *room,RoomDetail *detail);
void baseClientSendDetailTint(BaseRoom *room,RoomDetail *detail,Color colors[2]);
void baseClientSendPermissionUpdate(BaseRoom *room,RoomDetail *detail);

void baseClientSendRoomCreate(int size[2],Vec3 pos,F32 height[2],const RoomTemplate *info,RoomDoor *doors,int door_count);
void baseClientSendRoomMove(BaseRoom *room, int rotation, Vec3 pos,RoomDoor *doors,int door_count);
void baseClientSendRoomUpdate(int room_id,int size[2],Vec3 pos,F32 height[2],RoomDoor *doors,int door_count);
void baseClientSendRoomDelete(int room_id);

void baseClientSendPlot( const BasePlot * plot, Vec3 pos, int rot );
void baseClientSendMode( int mode);

void baseClientSendApplyStyleToBase(BaseRoom *room);
void baseClientSendApplyLightingToBase(BaseRoom *room);
void baseClientSendApplyRoomTintToBase(BaseRoom *room);

void baseClientSendUndo();
void baseClientSendRedo();

void baseClientSendDefaultSky(int defaultSky);

#endif
