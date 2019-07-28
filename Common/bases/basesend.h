#ifndef _BASESEND_H
#define _BASESEND_H

#include "Color.h"
typedef struct Packet Packet;
typedef struct RoomDetail RoomDetail;
typedef struct BaseRoom BaseRoom;
typedef struct RoomTemplate RoomTemplate;

typedef enum BaseSendType
{
	kBaseSend_Single,
	kBaseSend_Room,
	kBaseSend_Base,
}BaseSendType;

void baseSendDetails(Packet *pak,BaseRoom *room,RoomDetail **details,int count,int reset);
void baseSendDetailDelete(Packet *pak, BaseRoom *room,RoomDetail *detail);
void baseSendDetailMove(Packet *pak, BaseRoom *room,RoomDetail *detail, BaseRoom *newroom);
void baseSendDetailTint(Packet *pak, BaseRoom *room,RoomDetail *detail, Color *colors);
void baseSendPermissions(Packet *pak,BaseRoom *room,RoomDetail *detail);
void baseSendUndo(Packet *pak);
void baseSendRedo(Packet *pak);

// possibly client only
void baseSendHeight(Packet *pak,BaseRoom *room,int type,int idx,F32 floor,F32 ceiling);
void baseSendTexSwap(Packet *pak,BaseRoom *room,int idx,char *texname);
void baseSendGeoSwap(Packet *pak,BaseRoom *room,int idx,char *style);
void baseSendLight(Packet *pak,BaseRoom *room,int idx,Color light_color);
void baseSendDoorway(Packet *pak,BaseRoom *room,int pos[2],F32 height[2]);
void baseSendDoorwayDelete(Packet *pak,BaseRoom *room,int pos[2]);
void baseSendTint(Packet *pak,BaseRoom *room,int idx,Color *colors);

// possibly server only
typedef struct Base Base;
void baseSendAll(Packet *pak,Base *base,U32 mod_time,int full_update);
void baseSendRoomAll(Packet *pak,BaseRoom *room);

#endif
