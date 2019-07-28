#ifndef _CLOTHNODE_H
#define _CLOTHNODE_H

#include "model.h"
#include "gfxtree.h"

typedef enum ClothType {
	CLOTH_TYPE_CAPE = 1,
	CLOTH_TYPE_FLAG = 2,
	CLOTH_TYPE_TAIL = 3
} ClothType;

typedef struct SeqInst SeqInst;
typedef struct ClothWindInfo ClothWindInfo;
typedef struct ParticleSystem ParticleSystem;

// Pre calculated values for faster runtime performance
// Stored in ClothCapeData
typedef struct ClothBodyColInfo
{
	BoneId bonenum1;
	BoneId bonenum2;
} ClothBodyColInfo;
#define MAX_CLOTH_COL 12
#define MAX_NUM_LODS 3

//////////////////////////////////////////////////////////////////////////////
// NODE DATA
// This is game specific data stored with the ClothObject structure.
// NOTE: ClothCapeData is a superset of ClothNodeData and the overlapping
//  members must line up.
typedef struct ClothNodeData
{
	// Common
	U32 readycount;		// Number of frames cloth has been updated
	S32 type;			// = CLOTH_TYPE_*
	U32 frame;			// Last frame updated
	F32 avgdt;			// 1/Average framerate (only used if USE_AVGTIME = 1)
	int avgdtfactor;	// Number of elements averaged together so far
	const char *seqType;		// Sequencer type to use for collision
	ParticleSystem *system; // Particle system if this is being rendered in the particle rendering phase
} ClothNodeData;

typedef struct ClothCapeData
{
	// Common
	U32 readycount;		// Number of frames cloth has been updated
	ClothType type;			// = CLOTH_TYPE_CAPE
	U32 frame;			// Last frame updated
	F32 avgdt;			// 1/Average framerate (only used if USE_AVGTIME = 1)
	int avgdtfactor;	// Number of elements averaged together so far
	const char *seqType;		// Sequencer type to use for collision
	ParticleSystem *system; // Particle system if this is being rendered in the particle rendering phase
	// Cape specific
	Vec3 scale;			// Character scale factor
	F32 avgscale;		// ||scale||, used for scaling radii.
	// Bulky collision info storage... 
	ClothBodyColInfo colinfo[MAX_NUM_LODS][MAX_CLOTH_COL];
} ClothCapeData;



GfxNode * initTestClothNode( int * nodeId, char * texture, int type );
GfxNode * initClothCapeNode(int * nodeId, const char *textures[4], U8 colors[4][4], Model *cape, Model *harness, Vec3 scale, F32 stiffness, F32 drag, F32 point_y_scale, F32 colrad, ClothType clothType, const char *seqType, const char *trickName);
void updateClothNodeParams(GfxNode * node, const char *textures[4], U8 colors[4][4]);
void resetClothNodeParams(GfxNode *node);
void freeClothNode( ClothObject * clothdata );
void updateCloth(SeqInst * seq, GfxNode *clothnode, const Mat4Ptr worldmat, F32 camdist, int numhooks, Vec3 *hooks, Vec3 *hookNormals, const Vec3 positionOffset, ClothWindInfo *windInfoOverride);
void updateClothHack(SeqInst * seq, GfxNode *clothnode, Mat4Ptr worldmat, F32 camdist);
void hideCloth(GfxNode * clothnode);
void calculateClothWindThisFrame(void);
void updateClothFrame(void);
void loadClothWindInfo(void);
void loadClothColInfo(void);
ClothWindInfo *getWindInfo(const char *name);
const ClothWindInfo *getWindInfoConst(const char *name);

#endif
