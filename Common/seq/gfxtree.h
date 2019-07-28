#ifndef _GFXTREE_H
#define _GFXTREE_H

#include "animtrack.h"
#include "rt_state.h"
#include "texEnums.h"

//#include "splat.h"

#define CLOTH_HACK 1

typedef struct ClothObject ClothObject;
typedef struct TrickNode TrickNode;
typedef struct MiniTracker MiniTracker;
typedef struct Model Model;

typedef enum 
{
	ENTLIGHT_DONT_USE		= 0,
	ENTLIGHT_INDOOR_LIGHT	= ( 1 << 0 ),
	ENTLIGHT_PULSING		= ( 1 << 1 ),
	ENTLIGHT_CUSTOM_AMBIENT = ( 1 << 2 ),
	ENTLIGHT_CUSTOM_DIFFUSE	= ( 1 << 3 ),
	ENTLIGHT_USE_NODIR		= ( 1 << 4 ),
} eLightUse;

typedef struct EntLight
{
	U8		use;		//should this ent be using special light right now, or just the regular sunlight?
	U8		hasBeenLitAtLeastOnce; //whether to interpolate.
	
	Vec4	ambient;	//to use for this seq
	Vec4	diffuse;	//""
	Vec4	direction;	//""

	// Used for calculating the three above
	Vec3	tgt_ambient;	//for interpolating from one light to another
	Vec3	tgt_diffuse;	//""
	Vec3	tgt_direction;	//""
	F32		interp_rate;	//""

	F32		ambientScale;	//factor by which to scale ambient at all times
	F32		diffuseScale;	//""
	F32		pulseTime;

	F32 pulseEndTime;
	F32 pulsePeakTime;
	F32	pulseBrightness;
	F32	pulseClamp;

	Vec3	calc_pos;		//last pos of ent when light was calculated for it (optimization for lightEnt only)
} EntLight;

//If this gfxNode is a Shadow Node, then you need one of these. 
typedef struct
{
	int		triCount;	//number triagles in the shadow, no sharing of verts means vertCount = 3 * triCount
	int		vertCount;	//number triagles in the shadow, no sharing of verts means vertCount = 3 * triCount
	Vec3	* verts;		
	Vec2	* sts;	
	U8		* colors;
} Shadow;

/*
It is a structure that many gfxNodes can point to with info that is common to them all.  Right now every sequencer 
has one that all the gfxNodes created by that sequencer has a pointer to.   
Data in it right now is 
1. xforms of all the gfxNodes in it's animation relative to the hips.  Allows gfxNodes to use other gfxNodes as bones
2. calculated light info for the entity that gfxNodes on the entity need to use
*/
typedef struct SeqGfxData
{
	Vec3		btt[BONEID_COUNT];	// BoneTranslationTotal from the hips
	Mat4		bpt[BONEID_COUNT];	// BonePosition with btt pre-subtracted off
	Vec3		shieldpos;			// offset information for shield fx from costume parts (add more as needed)
	Vec3		shieldpyr;			// offset information for shield fx from costume parts (add more as needed)
	EntLight	light;				// Any special lighting this gfxnode should use
	U8			alpha;				// replace gfxtree set alpha
	void *		tray;				// Tray this animation is in currently
} SeqGfxData;

enum
{
	GFX_USE_SKIN = 1 << 0,
	GFX_USE_OBJ  = 1 << 1,
	GFX_USE_FX   = 1 << 2,
	GFX_USE_CHILD= 1 << 3
};

typedef enum TrickFrom
{
	TRICKFROM_UNKNOWN,
	TRICKFROM_MODEL,
} TrickFrom;

typedef struct GfxNode 
{
	//Hot
	int				seqHandle;  //used to check boneidex ptr, but that's sloppy
	U8				useFlags;
	struct GfxNode *next;

	//Warm
	struct GfxNode *child;
	U8				alpha;
	U16				flags;
	BoneId			anim_id; //bone that this gfx_node represents
	Model			*model;
	Mat4Ptr			viewspace;
	Mat4			mat;
	Vec3			bonescale;
	Vec3			animDelta;   //Different between baseSkeleton and animationTrack, for playing, say male anims on chicks

	//Cool
	void			* splat; //To do, Union with something?
#if CLOTH_HACK
	ClothObject		* clothobj; //To do, Union with something?
#endif
	const BoneAnimTrack   * bt;	//your animation track
	U8				fxRefCount;
	U8				trick_from_where;	// Where this node got a trick from (for reloading)
	char			*rgbs;
	int				unique_id; 
	SeqGfxData      * seqGfxData;  //Data shared by all gfxnodes in a seq. The SeqInst has a SeqGfxData
	U8				rgba[4];
	U8				rgba2[4];
	TrickNode		*tricks;
	// For old nodes/texturing:
	TextureOverride customtex;
	BlendModeType	node_blend_mode;
	// For new AVSN2:
	MiniTracker		*mini_tracker;

	//Cold
	Model           *shadow_model; //not used right now
	struct GfxNode *parent;	// these are accessed rarely, try to keep them
	struct GfxNode *prev;		// in a separate cache line

} GfxNode;

extern GfxNode * gfx_tree_root;
extern GfxNode * sky_gfx_tree_root;

static INLINEDBG int gfxTreeNodeIsValid(GfxNode * node, int id) 
{
	return node && node->unique_id == id;
}

void gfxTreeFindWorldSpaceMat(Mat4 result, GfxNode * node);
GfxNode *gfxTreeFindRecur(char *name,GfxNode *node);
GfxNode * gfxTreeFindBoneInAnimation(BoneId bone, GfxNode *node, int seqHandle, int root);
TrickNode * gfxTreeAssignTrick( GfxNode * node );
void gfxTreeInitGfxNodeWithObjectsTricks(GfxNode * node, TrickNode *trick);
void gfxTreeInitCharacterAndFxTree();
void gfxTreeInit();
#ifdef CLIENT
void gfxTreeInitSkyTree();
GfxNode *gfxTreeInsertSky(GfxNode *parent);
void gfxTreeDeleteSky(GfxNode *node);
#endif
GfxNode *gfxTreeInsert(GfxNode *parent);
void gfxTreeDelete(GfxNode *node);
void gfxTreeDeleteAnimation(GfxNode * node, int seqHandle);
int gfxTreeRelinkSuspendedNodes(GfxNode * node, int seqHandle);
void gfxTreeResetTrickFlags( GfxNode * gfxnode );
bool gfxTreeParentIsVisible(GfxNode * node);

#endif
