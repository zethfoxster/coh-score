/***************************************************************************
*     Copyright (c) 2000-2006, Cryptic Studios
*     All Rights Reserved
*     Confidential Property of Cryptic Studios
***************************************************************************/
#ifndef MENUAVATAR_H__
#define MENUAVATAR_H__

// camera states in the shell
typedef enum AvatarMode
{
	AVATAR_GS_FEMALE,
	AVATAR_GS_MALE,
	AVATAR_GS_HUGE,
	AVATAR_GS_BM,
	AVATAR_GS_BF,
	AVATAR_GS_ENEMY,
	AVATAR_GS_ENEMY2,
	AVATAR_GS_ENEMY3,
	AVATAR_DEFAULT_MALE,	// the default shell avatar position ( now only used in costume screen )
	AVATAR_DEFAULT_FEMALE,
	AVATAR_DEFAULT_HUGE,
	AVATAR_LOGIN,			// the login avatar position
	AVATAR_PROFILE_MALE,	// the male position in the profile screen to make him line up with ID card window
	AVATAR_PROFILE_FEMALE,  // same for female
	AVATAR_PROFILE_HUGE,	// same for huge
	AVATAR_CENTER_MALE,
	AVATAR_CENTER_FEMALE,
	AVATAR_CENTER_HUGE,
	AVATAR_E3SCREEN,
	AVATAR_KOREAN_COSTUMECREATOR_SCREEN,
}AvatarMode;

enum
{
	ZOOM_DEFAULT,	// zoomed out all the way
	ZOOM_IN,		// close up of the face
};

extern int gZoomed_in;

typedef struct Entity Entity;

void moveAvatar( AvatarMode mode );
// moves the avatar to the correct position

F32* avatar_getDefaultPosition(int costumeEditMode, int centered);
F32* avatar_getZoomedInPosition(int costumeEditMode, int centered);

void zoomAvatar(bool init, Vec3 destPos);
void scaleAvatar();

#endif /* #ifndef MENUAVATAR_H__ */

/* End of File */
