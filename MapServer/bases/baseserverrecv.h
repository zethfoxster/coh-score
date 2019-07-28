#ifndef BASESERVERRECV_H
#define BASESERVERRECV_H

#include "stdtypes.h"

typedef struct Packet Packet;
typedef struct Entity Entity;
typedef struct RoomTemplate RoomTemplate;
typedef struct Base Base;
typedef struct BasePlot BasePlot;
typedef struct RoomDoor RoomDoor;

int baseReceiveEdit(Packet *pak,Entity *e);
int roomUpdate(Entity * e, int room_id,int size[2],Vec3 pos,F32 height[2], const RoomTemplate *info,RoomDoor *doors,int door_count);
void plotUpdate(Entity * e, const BasePlot *plot, Vec3 pos );
void baseCreateDefault(Base *base);

#endif