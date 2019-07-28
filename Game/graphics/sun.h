#ifndef _SUN_H
#define _SUN_H

#include "mathutil.h"
#include "model.h"
#include "groupscene.h"

#define SUN_INITIALIZE	1

typedef struct GfxNode GfxNode;

typedef struct SkyNode {
	GfxNode *node;
	F32 alpha;
	bool snap;
} SkyNode;
typedef struct SkyNodeList {
	SkyNode *nodes;
	int max, count;
} SkyNodeList;

typedef struct SunLight
{
	Vec4	diffuse;
	Vec4	diffuse_reverse;		// 12/2009 - deprecated
	Vec4	ambient;
	Vec4	ambient_for_players;	//entities get an extra bit of lovin
	Vec4	diffuse_for_players;	//""
	Vec4	diffuse_reverse_for_players;	// 12/2009 - deprecated
	Vec4	no_angle_light;			//ambient + diffuse
	F32		luminance;
	F32		bloomWeight;
	F32		toneMapWeight;
	F32		desaturateWeight;

	Vec4	direction;				//direction of the sun's light
	Vec4	direction_in_viewspace;	//for convenience
	Vec3	shadow_direction;		//direction of shadow's light (prevented from ever being right overhead becasue of tree stencils
	Vec3	shadow_direction_in_viewspace;//for convenience
	U32		fixed_position : 1;

	F32		gloss_scale;			//bumpmapping level set in sky file
	Vec3	position;				//of the sun. Used by bumpmapping
	F32		lamp_alpha;				//how bright the lamp glow should be.  Set in sky file.
	U8		lamp_alpha_byte;		//how bright the lamp glow should be.  Set in sky file.
	Vec4	shadowcolor;			//shadow color and alpha. Set in sky file

	Vec2	fogdist;
	Vec3	fogcolor;
	Vec3	bg_color;

	DOFValues	dof_values;			// Depth of Field values

	SkyNodeList sky_node_list;

	// Need to update SunLightParseInfo when adding things to this struct
} SunLight;

typedef struct FogVals FogVals;

bool isIndoors(void);

F32 fixTime(F32 a);
void sunVisible(void);
void sunGlareDisable(void);
void sunGlareEnable(void);
void skyInvalidateFogVals(void);
void skySetIndoorFogVals(int fognum,F32 fog_near,F32 fog_far,U8 color[4], FogVals *fog_vals);
void sunUpdate(SunLight *sun, int init);
void skyLoad( char ** skyFileNames, char *sceneFileName );
void skyNotifyGfxTreeDestroyed(void);
TexSwap **sceneGetTexSwaps(int *count);
TexSwap **sceneGetMaterialSwaps(int *count);
ColorSwap **sceneGetColorSwaps(int *count);
void sceneAdjustColor(Vec3 rgb_in,Vec3 rgb);
F32 *sceneGetAmbient(void);
F32 *sceneGetMinCharacterAmbient(void);
F32 *sceneGetCharacterAmbientScale(void);
void sunPush(void);
void sunPop(void);

typedef enum DOFSwitchType {
	DOFS_SNAP,
	DOFS_SMOOTH,
} DOFSwitchType;

#define DOF_LEAVE (-1.f)

void setCustomDepthOfField(DOFSwitchType type, F32 focusDistance, F32 focusValue, F32 nearDist, F32 nearValue, F32 farDist, F32 farValue, F32 duration, SunLight *sun);
void unsetCustomDepthOfField(DOFSwitchType type, SunLight *sun);

void sunSetSkyFadeClient(int sky1, int sky2, F32 weight); // 0.0 = all sky1
void sunSetSkyFadeOverride(int sky1, int sky2, F32 weight); // Override for volume triggers
void sunUnsetSkyFadeOverride(void);

extern SunLight g_sun;

#endif
