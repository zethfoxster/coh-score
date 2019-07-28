#ifndef _CAMERA_H
#define _CAMERA_H

#include "stdtypes.h"
#include "mathutil.h"

#define	TICKS_FOR_COMBAT_CAM_TRANSITIONS	( 15 )

typedef struct DefTracker DefTracker;
typedef struct GfxNode GfxNode;

typedef struct CameraInfo
{
	Mat4	mat;			//Camera's target, used to derive cammat
	Mat4	cammat;			//Camera's actual position and orientation
	Mat4	viewmat;		//cammat transposed and jiggered with.  Use to transform world space to camera space.
	Mat4	inv_viewmat;	//viewmat inverted

	Vec4	view_frustum[6];	//view frustum planes

	Vec3	pyr;			//Current PYR
	Vec3	targetPYR;		//What PYR is the camera trying to get to?
	float	rotate_scale;	//Eases camera move from pyr to targetPYR when you are automatically rotated to face your attack target

	bool    camIsLocked;	
	
	float	targetZDist;    //game_state.camdist
	float	lastZDist;		//current Z Dist 
	F32		zDistImMovingToward; //if last Z dist is camera moving on a spring, this is where it's headed
	F32		camDistLastFrame; //so you can know if the scroll wheel has ticked back.
	F32		triedToScrollBackThisFrame; // so if you want to scroll back past an obstruction, it's uses the same rules it did to move forward
	F32		CameraYLastFrame;
	

	DefTracker	*last_tray;	// Tray the camera was in the last time it was checked (every frame not in the editor)
							//(Used by lighting code, not the camera code)

	int		simpleShadowUniqueId;
	GfxNode	* simpleShadow;

} CameraInfo;

extern CameraInfo	cam_info;

void camSetPyr(Vec3 pyr);  
void camSetPos(const Vec3 pos, bool interpolate );
void camLockCamera( bool isLocked );
void camUpdate();	
void camPrintInfo();
float camGetPlayerHeight();

void toggleDetachedCamControls(bool on, CameraInfo *ci);	// camera_detached.c

#endif
