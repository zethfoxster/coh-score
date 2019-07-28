/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef UIBASEINPUT_H__
#define UIBASEINPUT_H__

#include "basedata.h"

typedef enum DragType
{
	kDragType_None = 0,
	kDragType_Drag,
	kDragType_Sticky,
} DragType;

typedef struct BaseEditState 
{
	int forward;
	int backward;
	int left;
	int right;
	int turnleft;
	int turnright;
	int mouselook;
	int zoomin;
	int zoomout;
	int dragging;
	float gridsnap;
	float anglesnap;
	int roomclip;
	AttachType nextattach;

	DragType surface_dragging;
	DragType detail_dragging;
	DragType room_dragging;
	DragType door_dragging;
	DragType plot_dragging;
	DragType room_moving;

} BaseEditState;

extern BaseEditState baseEdit_state;
extern bool s_bLooking;

typedef struct KeyBindProfile KeyBindProfile;
extern KeyBindProfile baseEdit_binds_profile;

void baseEditKeybindInit(void);
void baseEditToggle(int iOn);
void baseSetLocalMode(int mode);

#endif /* #ifndef UIBASEINPUT_H__ */

/* End of File */

