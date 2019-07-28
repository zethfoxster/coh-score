#ifndef _BASELEGAL_H
#define _BASELEGAL_H

typedef struct Base Base;
typedef struct BaseRoom BaseRoom;
typedef struct RoomDetail RoomDetail;

void rotateCoordsIn2dArray(int coords[2], int array_size[2], int rotation, int reverse);

void baseToDetailBlocks(Base *base);
int roomDetailLegal(BaseRoom *room,RoomDetail *detail, int *error);
int roomBlockLegal(BaseRoom *room,int change_pos[2],Vec2 change_height);
int roomDoorwayLegal(BaseRoom *room,int block[2]);
BaseRoom *getLegalDoorway( Base *base, BaseRoom *room,int block[2],F32 height[2], int neighbor_block[2] );

BaseRoom *setDoorway(Base *base,BaseRoom *room,int block[2],F32 height[2], int auto_alloc);
void checkAllDoors(Base *base,int auto_alloc);

void addRoomDoors(Base *base,BaseRoom *room,RoomDoor ***doors);
void deleteRoomDoors(Base *base,BaseRoom *room);

bool basePlotFit(Base *base, int w, int h, Vec3 pos, int update);
bool addOrChangeRooomIsLegal( Base * base, int room_id, int size[2], int rotation, Vec3 pos, 
							 F32 ht[2], const RoomTemplate *info, RoomDoor *doors, int door_count );

int baseIsLegal(Base *base, int *error);
char *getBaseErr(int kBaseErrCode);

#endif
