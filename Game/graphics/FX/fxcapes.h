#ifndef FXCAPES_H
#define FXCAPES_H

#include "stdtypes.h"
#include "gfxtree.h"
#include "clothnode.h"

typedef struct ClothWindInfo ClothWindInfo;

typedef struct FxCape
{
	struct		FxCape * next; //development only for listing non binary loaded bhvrs
	struct		FxCape * prev;

	char*		name;
	char*		geoName;
	char*		geoFile;
	char*		harnessName;
	char*		harnessFile;
	char*		trickName;
	char*		texture[4];
	U8			rgb[4][3];

	int			fileAge;		// development only

	F32			stiffness;
	F32			drag;
	F32			scale;
	Vec2		scaleXY;
	F32			cape_lod_bias;
	F32			point_y_scale;
	F32			colrad;

	bool			isflag;
	char		*windInfoName;
	ClothWindInfo	*windInfo;
	char		*colInfoName; // Override for collision info (instead of using ent type name)
} FxCape;

typedef struct FxParams FxParams;
//typedef enum ClothType ClothType;
typedef struct ParseTable ParseTable;
#define TokenizerParseInfo ParseTable

typedef struct FxCapeInst
{
	// Pointer to general configuration
	FxCape*		cape;
	int			capeFileAge;		// development only

	// Run-time/per-instance
	GfxNode*	clothnode;
	int			clothnodeUniqueId;
	Model*		capeGeom;
	Model*		capeHarness;
	int			pathToSkin[2]; // bitmask list of decisions to use to skin the appropriate bones
	int			pathToSkinCalculated;
	U8			rgba[4][4];		
	U8			dumb[4][4];		
	ClothType	type;
	int			updateTimestamp;
} FxCapeInst;

FxCapeInst *createFxCapeInst(void);
void destroyFxCapeInst(FxCapeInst *fxCapeInst);

void fxPreloadCapeInfo(void);
bool fxCapeInitWindInfo(TokenizerParseInfo *tpi, void *structptr);
void fxCapeInstInit(FxCapeInst *fxCapeInst, char *capeFile, int seqHandle, FxParams *fxp);
void fxCapeInstChangeParams(FxCapeInst *fxCapeInst, FxParams *fxp);
int fxCapeGeoExists(const char * harnessName, char cape_name[]);

#endif