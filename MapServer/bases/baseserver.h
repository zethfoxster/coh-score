#ifndef BASESERVER_H
#define BASESERVER_H

typedef struct Entity Entity;
typedef struct RoomDetail RoomDetail;
typedef struct RoomTemplate RoomTemplate;
typedef struct Base Base;
typedef struct BaseRoom BaseRoom;
typedef struct BasePlot BasePlot;
typedef struct RoomDoor RoomDoor;
typedef struct StaticMapInfo StaticMapInfo;
typedef struct Supergroup Supergroup;

#define BASE_UNDO_LEVELS 8

void baseEditFailed( Entity *e, char * str );
	// if any of bruces checks fail, this will send player generic error msg

int baseBuyPlot( Entity * e, const BasePlot * old_plot, const BasePlot * new_plot, int rot );
	// will purchase item using supergroup funds, returns true on success

int baseBuyRoomDetail( Entity *e, RoomDetail * detail, BaseRoom * room );
	// will purchase item using supergroup funds, returns true on success

int baseSellRoomDetail( Entity * e, RoomDetail * detail, BaseRoom * room  );
	// will refund cost of item to supergroup funds, true on success

int baseBuyRoom( Entity *e, int room_id, const RoomTemplate * room );
	// will purchase room using supergroup funds, returns true on success

int baseSellRoom( Entity * e, BaseRoom *room );
	// will refund cost of room to supergroup funds, true on success

void baseCheckpointPrestigeSpent();

int baseUndoClear();
int baseUndoCount();
int baseUndoPush();
int baseUndoPop();
int baseUndo();
int baseRedoCount();
int baseRedo();

int getSafeEntrancePos( Entity *e, Vec3 pos, int isDoor, Vec3 door_pos );
void teleportEntityInVolumeToPos( Vec3 min, Vec3 max, Vec3 dest, int not_architect );
void teleportEntityInVolumeToEntrance( Vec3 min, Vec3 max, int not_architect, int isDoor, Vec3 door_pos  );
void teleportEntityInVolumeToClosestDoor( BaseRoom *room, Vec3 min, Vec3 max, int not_architect, int isDoor, Vec3 door_pos );
void teleportEntityInVolumeToHt( Vec3 min, Vec3 max, float ht );
void teleportEntityToNearestDoor( Entity *e );
	// These functions are used to move players around if they are either trapped in volume of newly created
	// object or in the volume of a deleted object.

void checkWalledOffAreasForPlayers( BaseRoom *room );
void baseSetBlockHeights( Entity * e, BaseRoom *room, float height[2], int idx);
void baseSetRoomHeights( Entity * e, BaseRoom *room, float height[2] );
void baseSetBaseHeights( Entity * e, float height[2] );
	// These functions are used to set the floor and ceiling heights

void baseApplyRoomSwapsToBase( BaseRoom * master_room );
void baseApplyRoomTintsToBase( BaseRoom * master_room );
void baseApplyRoomLightToBase( BaseRoom * master_room );
	// applies settings from the room to entire base

void sendActivateUpdate(Entity *player, bool activate, int detailID, int interactType);
void sendBaseMode(Entity *player, int mode);
void sendRaidState(Entity *player, int mode);
void baseActivateDetail(Entity *e, int detailID, char *group);
void baseDeactivateDetail(Entity *e, int type);
void base_Tick(Entity *e);
    // Handle players interacting with base details
Entity *baseDetailInteracting(RoomDetail *det);
    // Return Entity interacting with given room detail (or none)
const char ** baseGetMapDestinations(int detailID);
void completeBaseGurneyXferWork(Entity *e, int detailID);

void sendSGPrestige(Entity *e);
void applySGPrestigeSpent(Supergroup *sg);
int baseGetAndClearSpent(void);
void applyEntInfluenceSpent(Entity *e, int influenceSpent);

int getNumberOfDoorways( void );
void sendPlayerToDoorway(Entity *e, int doorwayidx, int debug_print);


#endif