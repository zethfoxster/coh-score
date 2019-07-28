/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef BASEEDIT_H__
#define BASEEDIT_H__

#include "basedata.h"

#define DECOR_FADE_HOLD 0
#define DECOR_FADE_OUT 0.8
#define DECOR_FADE_IN 0.2
#define DECOR_FADE_TOTAL (DECOR_FADE_IN+DECOR_FADE_HOLD+DECOR_FADE_OUT)

typedef struct BaseRoom BaseRoom;
typedef struct RoomDetail RoomDetail;
typedef struct BaseBlock BaseBlock;
typedef struct RoomTemplate RoomTemplate;
typedef struct Detail Detail;
typedef struct BasePlot BasePlot;
typedef struct RoomDoor RoomDoor;
typedef struct DefTracker DefTracker;
typedef struct GroupDef GroupDef;

typedef struct BaseRoomRef
{
	BaseRoom *p;
	int id;
} BaseRoomRef;

typedef struct BaseBlockRef
{
	BaseBlock *p;
	int block[2];
	int iDecor;
	Mat4 mat;
} BaseBlockRef;

typedef struct RoomDetailRef
{
	BaseRoomRef refRoom;

	RoomDetail *p;
	int id;
	int iNumAttachedAux;

} RoomDetailRef;


extern BaseBlockRef g_refCurBlock;
extern BaseRoomRef g_refCurRoom;
extern RoomDetailRef g_refCurDetail;
extern RoomDetailRef g_refHoverDetail;
extern const RoomTemplate *g_pDragRoomTemplate;
extern int g_iXDragStart;
extern int g_iYDragStart;
extern int g_iDecor;
extern int g_iDecorFadeTimer;

int baseedit_Mode(void);
	// Returns true if in base editing mode.

void baseedit_Process(void);
	// The main processing loop for the base editor.

void baseedit_SelDraw(void);
	// Draws the current highlights and selections for the base editor.
	//   (Called after the rest of the scene is drawn.)



void baseedit_SetCurRoom(BaseRoom *pRoom, int block[2]);
	// Sets the current room and/or block within the room. NULL can be
	//   passed pRoom or block if they aren't being changed.

void baseedit_SetCurDetail(BaseRoom *pRoom, RoomDetail *pDetail);
	// Sets the current detail being manipulated.

void baseedit_SetHoverDetail(BaseRoom *pRoom, RoomDetail *pDetail);
	// Sets the current detail being hovered over by the mouse pointer.

void baseedit_SetCurDecor(int iDecor);
	// Sets the current decor type.

void baseedit_StartRoomDrag(int iXSize, int iZSize, const RoomTemplate *info);
	// Starts a room placement drag for a room of the given size.

void baseedit_StartRoomMove(BaseRoom * room );
// Starts a room placement drag for a room of the given size.

void baseedit_CreateRoom(const RoomTemplate *pinfo, int size[2], Vec3 pos, F32 afHeights[2], RoomDoor * doors, int door_count );
	// Creates a room.

void baseedit_MoveRoom(BaseRoom *room, int rotation, Vec3 pos, RoomDoor * doors, int door_count );
	// Moves a Room.

void baseedit_RotateRoom(void);
	// Switches the orientation of the room being dragged. (Only useful
	//   when in room drag mode.)

void baseedit_DeleteRoom(BaseRoom *pRoom);
	// Delete the given room

void baseedit_CreateDoorway(BaseRoom *pRoom, int block[2], F32 afHeights[2]);
	// Creates a door

void baseedit_StartDetailDrag(BaseRoom *pRoom, RoomDetail *pDetail, int dragtype);
	// Starts a detail placement drag for the given RoomDetail.

void baseedit_StartSurfaceDrag(int iSurface);
	// Starts a surface (floor or ceiling) drag for changing their height.

void baseedit_StartDoorDrag();
	// starts a door drag, for moving a door

void baseedit_StartPlotDrag( int type, const BasePlot * plot );
// starts a door drag, for moving a door

void baseedit_Select(int x, int y);
	// Handles what is typically a mouse click, using the current mouse
	//   location. Might select an object, or room location, or place
	//   a new room, etc. depending on the current mode.

void baseedit_SetCamDist(void);
	// Sets the camera distance based on the current mousewheel position.

void baseedit_SetCamDistAngle(float dist, float angle);
	// Sets the camera distance based on the current mousewheel position.

bool baseedit_InDragMode(void);
	// Returns if in a drag mode (sticky or actual drag)

bool baseedit_CancelDrags(void);
	// Cancels any pending drag operations, if any. Returns true if there
	//   were any drag operations pending.

bool baseedit_CancelSelects(void);
	// Clears and selections, if any. Returns true if there were any 
	//   selections.

void baseedit_SyncUIState(void);
	// Makes sure that the current room, object, etc. are still valid after
	//   an update from the server.

void baseedit_InvalidateGroupDefs(void);
	// Clear out old GroupDefs after a server reset

void baseedit_CopyMatFromBlock(BaseRoom *pRoom, int block[2], int iSurface, Mat4 mat);
	// Copies the matrix for the given room and block and surface
	//   (floor/ceiling) into mat.

void baseedit_LookAtRoomBlock(BaseRoom *pRoom, int block[2]);
	// Moves the camera so the given room and block are centered in the view.



void baseedit_CreateDetail(BaseRoom *pRoom, const Detail *pDetail, Mat4 mat, Mat4 editor_mat, Mat4 surface_mat, int invID, AttachType attach);
	// Creates a new detail with the given geometry and location.

void baseedit_DeleteDetail(BaseRoom *pRoom, RoomDetail *pDetail);
	// Deletes the given detail from the room.

void baseedit_MoveDetail(BaseRoom *pRoom, RoomDetail *pDetail, Mat4 mat, Mat4 editor_mat, Mat4 surface_mat);
	// Moves the given detail in the given room to the given location.

void baseedit_MoveDetailToRoom(BaseRoom *pRoom, RoomDetail *pDetail, BaseRoom *pNewRoom, Mat4 mat, Mat4 editor_mat, Mat4 surface_mat);
// Moves the given detail in the given room to the given location in a new room

void baseedit_RotateDetail(BaseRoom *pRoom, RoomDetail *pDetail, int clockwise );
	// Rotates the detail 90 degrees in specified direction

void baseedit_SetDetailTint( BaseRoom *pRoom, RoomDetail *pDetail, int c1, int c2 );
	// Sets the color for the given detail in the room.

void baseedit_SetDetailPermissions(BaseRoom *pRoom, RoomDetail *pDetail);
	// Sets permissions for storage bins

void baseedit_RotateDraggedDetail(int clockwise);
	// Rotates the currently dragged detail.

void baseedit_SetRoomLight(BaseRoom *pRoom, int which, Vec3 color);
	// Sets the light color for the room.

void baseedit_SetRoomTexture(BaseRoom *pRoom, int partIdx, char *pchTexture);
	// Sets the texture for the given parts in the room.

void baseedit_SetRoomGeometry(BaseRoom *pRoom, int partIdx, char *pchGeometry);
	// Sets the geometry for the given parts in the room.

void baseedit_SetRoomTint( BaseRoom *pRoom,int partIdx, int c1, int c2 );
	// Sets the color for the given part in the room.

void baseedit_SetBlockHeights(BaseRoom *pRoom, int block[2], int height[2]);
	// Sets the floor and ceiling heights for a block.

void baseedit_SetRoomHeights(BaseRoom *pRoom,int height[2]);
	// set floor and cieling heaights for room

void baseedit_SetBaseHeights(int height[2]);
	// sets floor and cieling heights for entire base

void baseedit_LookAtCur(void);
	// goto cur detail if applicable, otherwise cur room

void baseedit_Rotate(int clockwise);
	// rotate room or item, whichever is aplicable

void baseedit_DeleteDoorway( BaseRoom * pRoom, int block[2]);
	// delete the currently selected doorway of the room (if it is a doorway)


int baseedit_GetDetailFromChild(DefTracker *tracker, RoomDetail **ppDetail);
	// Given a tracker pointing at some child piece, return the detail ID
	//   of the detail (or 0 if there isn't one)

int baseedit_IsClickableDetailEnt(Entity *ent);
	// return true if this entity corresponds to a clickable detail.

int baseedit_IsPassiveDetailEnt(Entity *ent);
	// returnt rue if this entity corresponds to a passive detail

void baseedit_SelectNext(bool bBackwards);
	// Select the next or previous visible base detail

void baseedit_SetDetailAlpha(RoomDetail *pDetail);
	// Turns on alpha when an item is being dragged

void baseedit_ClearDetailAlpha(RoomDetail *pDetailToIgnore);
	// Turns off alpha on any item except the parameters (may be NULL)

void baseCenterXY( int x, int y );
	// center camera on coordinates

bool FindReasonableSpot(BaseRoom *pRoom, RoomDetail *pDetail, const Mat4 editor_mat, const Mat4 surface, Vec3 vecGoodWorld, Vec3 vecGoodRoom, bool world);

// Some convenience functions which do the above, but on the
// current room.
static INLINEDBG void baseedit_LookAtCurRoomBlock(void)
		{ if(g_refCurRoom.p) baseedit_LookAtRoomBlock(g_refCurRoom.p, g_refCurBlock.block); }

static INLINEDBG void baseedit_MoveCurDetail(Mat4 mat, Mat4 editor_mat, Mat4 surface_mat)
		{ if(g_refCurRoom.p && g_refCurDetail.p) baseedit_MoveDetail(g_refCurRoom.p, g_refCurDetail.p, mat, editor_mat, surface_mat); }

static INLINEDBG void baseedit_RotateCurDetail(int clockwise)
		{ if(g_refCurRoom.p && g_refCurDetail.p) baseedit_RotateDetail(g_refCurRoom.p, g_refCurDetail.p, clockwise); }

static INLINEDBG void baseedit_SetCurDetailTint(int color1, int color2)
		{ if(g_refCurRoom.p && g_refCurDetail.p) baseedit_SetDetailTint(g_refCurRoom.p, g_refCurDetail.p, color1, color2); }

static INLINEDBG void baseedit_DeleteCurDetail(void)
		{ if(g_refCurRoom.p && g_refCurDetail.p) baseedit_DeleteDetail(g_refCurRoom.p, g_refCurDetail.p); }

static INLINEDBG void baseedit_SetCurRoomLight(int which, Vec3 color)
		{ if(g_refCurRoom.p) baseedit_SetRoomLight(g_refCurRoom.p, which, color); }

static INLINEDBG void baseedit_SetCurRoomTexture(int partIdx, char *pchTexture)
		{ if(g_refCurRoom.p) baseedit_SetRoomTexture(g_refCurRoom.p, partIdx, pchTexture); }

static INLINEDBG void baseedit_SetCurRoomGeometry(int partIdx, char *pchGeometry)
		{ if(g_refCurRoom.p) baseedit_SetRoomGeometry(g_refCurRoom.p, partIdx, pchGeometry); }

static INLINEDBG void baseedit_SetCurRoomTint(int partIdx, int color1, int color2)
		{ if(g_refCurRoom.p) baseedit_SetRoomTint(g_refCurRoom.p, partIdx, color1, color2); }

static INLINEDBG void baseedit_SetCurBlockHeights(int height[2])
		{ if(g_refCurRoom.p) baseedit_SetBlockHeights(g_refCurRoom.p, g_refCurBlock.block, height); }

static INLINEDBG void baseedit_DeleteCurRoom(void)
		{ if(g_refCurRoom.p) baseedit_DeleteRoom(g_refCurRoom.p); }

static INLINEDBG void baseedit_DeleteCurDoorway(void)
		{ if (g_refCurRoom.p) baseedit_DeleteDoorway(g_refCurRoom.p, g_refCurBlock.block);}

void basedit_Clear();

char *detailGetDescription(const Detail *detail,int longDesc);
void baseedit_AddSGSwap(GroupDef *def);

#endif /* #ifndef BASEEDIT_H__ */

/* End of File */

