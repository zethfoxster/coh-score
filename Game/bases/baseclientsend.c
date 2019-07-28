#include "bases.h"
#include "basesend.h"
#include "baseclientsend.h"
#include "clientcomm.h"
#include "entVarUpdate.h"
#include "mathutil.h"
#include "cmdgame.h"
#include "netcomp.h"
#include "uiConsole.h"
#include "basedata.h"

#include "edit_select.h"
#define START_BASE_PACKET START_INPUT_PACKET(pak, CLIENTINP_BASE_EDIT);
#define END_BASE_PACKET END_INPUT_PACKET; editWaitingForServer(1, 0);;

int baseWaitingForServer()
{
	return editIsWaitingForServer();
}

static genUniqueId(int *id)
{
	if (!*id)
		*id = ++g_base.curr_id;
}

void baseClientSendHeight(BaseRoom *room, int type, int idx,F32 floor,F32 ceiling)
{
	START_BASE_PACKET
	baseSendHeight(pak,room,type,idx,floor,ceiling);
	END_BASE_PACKET
}

void baseClientSendTexSwap(BaseRoom *room,int partIdx,char *texname)
{
	if( partIdx < -1 || partIdx >= ROOMDECOR_MAXTYPES )
		return;

	START_BASE_PACKET
	baseSendTexSwap(pak,room,partIdx,texname);
	END_BASE_PACKET
}

void baseClientSendGeoSwap(BaseRoom *room,int partIdx,char *style)
{
	if( partIdx < -1 || partIdx >= ROOMDECOR_MAXTYPES)
		return;

	START_BASE_PACKET
	baseSendGeoSwap(pak,room,partIdx,style);
	END_BASE_PACKET
}

void baseClientSendTint(BaseRoom *room,int partIdx,Color colors[2])
{
	if( partIdx < -1 || partIdx >= ROOMDECOR_MAXTYPES )
		return;

	START_BASE_PACKET
	baseSendTint(pak,room,partIdx,colors);
	END_BASE_PACKET
}

void baseClientSendLight(BaseRoom *room,int idx,Color light_color)
{
	START_BASE_PACKET
	baseSendLight(pak,room,idx,light_color);
	END_BASE_PACKET
}

void baseClientSendDoorwayDelete(BaseRoom *room,int block[2])
{
	START_BASE_PACKET
	baseSendDoorwayDelete(pak,room,block);
	END_BASE_PACKET
}

void baseClientSendDoorway(BaseRoom *room,int block[2],F32 heights[2])
{
	START_BASE_PACKET
		baseSendDoorway(pak,room,block,heights);
	END_BASE_PACKET
}


static void baseClientSendDetail(BaseRoom *room,RoomDetail *detail, BaseRoom *newroom)
{
	genUniqueId(&detail->id);
	detail->parentRoom = room->id;
	START_BASE_PACKET
	if (newroom)
	{
		baseSendDetailMove(pak,room,detail,newroom);
	}
	else
	{
		baseSendDetails(pak,room,&detail,1,0);
	}	
	END_BASE_PACKET
}

void baseClientSendDetailCreate(BaseRoom *room,RoomDetail *detail)
{
	detail->id = 0;
	baseClientSendDetail(room,detail,0);
}

void baseClientSendDetailUpdate(BaseRoom *room,RoomDetail *detail,BaseRoom *newroom)
{
	assert(detail->id);
	baseClientSendDetail(room,detail,newroom);
}

void baseClientSendDetailTint(BaseRoom *room,RoomDetail *detail,Color colors[2])
{
	START_BASE_PACKET
	baseSendDetailTint(pak,room,detail,colors);
	END_BASE_PACKET
}

void baseClientSendDetailDelete(BaseRoom *room,RoomDetail *detail)
{
	START_BASE_PACKET
	baseSendDetailDelete(pak, room, detail);
	END_BASE_PACKET
}

void baseClientSendPermissionUpdate(BaseRoom *room,RoomDetail *detail)
{
	genUniqueId(&detail->id);
	detail->parentRoom = room->id;
	START_BASE_PACKET
	baseSendPermissions(pak,room,detail);
	END_BASE_PACKET
}

void baseClientSendUndo()
{
	START_BASE_PACKET
	baseSendUndo(pak);
	END_BASE_PACKET
}

void baseClientSendRedo()
{
	START_BASE_PACKET
	baseSendRedo(pak);
	END_BASE_PACKET
}

void baseClientSendDefaultSky(int defaultSky)
{
	START_BASE_PACKET
		pktSendBitsPack(pak,1,BASENET_DEFAULT_SKY);
		pktSendBitsPack(pak,1,defaultSky);
	END_BASE_PACKET
}

static void baseClientSendRoom(int room_id,int size[2],Vec3 pos,F32 height[2],const RoomTemplate *info,RoomDoor *doors,int door_count)
{
	int	i = room_id;

	genUniqueId(&room_id);

	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_ROOM);
	pktSendBitsPack(pak,1,room_id);
	pktSendBitsPack(pak,1,size[0]);
	pktSendBitsPack(pak,1,size[1]);
	pktSendVec3(pak,pos);
	pktSendBitsPack(pak,1,(int)height[0]);
	pktSendBitsPack(pak,1,(int)height[1]);
	if(info)
		pktSendString(pak,info->pchName);
	else
		pktSendString(pak,"");
	pktSendBitsPack(pak,1,game_state.base_block_size);
	pktSendBitsPack(pak,1,door_count);
	for(i=0;i<door_count;i++)
	{
		pktSendBitsArray(pak,sizeof(RoomDoor)*8,&doors[i]);
	}
	END_BASE_PACKET
}

void baseClientSendRoomMove(BaseRoom * room, int rotation, Vec3 vec, RoomDoor *doors, int door_count)
{
	int i;
	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_ROOM_MOVE);
	pktSendBitsPack(pak,1,room->id);
	pktSendBitsPack(pak,1,rotation);
	pktSendVec3(pak,vec);
	pktSendBitsPack(pak,1,door_count);
	for(i=0;i<door_count;i++)
	{
		pktSendBitsArray(pak,sizeof(RoomDoor)*8,&doors[i]);
	}
	END_BASE_PACKET
}





void baseClientSendRoomDelete(int room_id)
{
	genUniqueId(&room_id);
	START_BASE_PACKET
		pktSendBitsPack(pak,1,BASENET_ROOM_DELETE);
		pktSendBitsPack(pak,1,room_id);
	END_BASE_PACKET
}

void baseClientSendRoomCreate(int size[2],Vec3 pos,F32 height[2],const RoomTemplate *info,RoomDoor *doors,int door_count)
{
	baseClientSendRoom(0,size,pos,height,info,doors,door_count);
}

void baseClientSendRoomUpdate(int room_id,int size[2],Vec3 pos,F32 height[2],RoomDoor *doors,int door_count)
{
	assert(room_id);
	baseClientSendRoom(room_id,size,pos,height,NULL,doors,door_count);
}

void baseClientSendPlot( const BasePlot * plot, Vec3 pos, int rot )
{
	START_BASE_PACKET
 	pktSendBitsPack(pak,1,BASENET_PLOT);
	pktSendString( pak, plot->pchName );
	pktSendF32( pak, pos[0] );
	pktSendF32( pak, pos[2] );
	pktSendBits(pak, 1, rot );
	END_BASE_PACKET
}

void baseClientSendMode(int mode )
{
	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_MODE);
	pktSendBitsPack(pak,1,mode);
	END_BASE_PACKET
}

void baseClientSendApplyStyleToBase(BaseRoom *room)
{
	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_BASESTYLE);
	pktSendBitsPack(pak,1,room->id);
	END_BASE_PACKET
}

void baseClientSendApplyLightingToBase(BaseRoom *room)
{
	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_BASELIGHTING);
	pktSendBitsPack(pak,1,room->id);
	END_BASE_PACKET
}

void baseClientSendApplyRoomTintToBase(BaseRoom *room)
{
	START_BASE_PACKET
	pktSendBitsPack(pak,1,BASENET_BASETINT);
	pktSendBitsPack(pak,1,room->id);
	END_BASE_PACKET
}

