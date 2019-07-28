#ifndef _TEX_H
#define _TEX_H 

#include "stdtypes.h"
#include <stdio.h>
#include "rt_state.h"
#include "texEnums.h"


#define MAX_TEX_SIZE 1024
#define MAX_MIP_CACHE_SIZE 128 // Largest size of mip map that could be cached/preloaded (actual size is probably 8x8)

#define DDS_FOURCC 0x00000004  // DDPF_FOURCC
#define DDS_RGB    0x00000040  // DDPF_RGB
#define DDS_RGBA   0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS



// The structure defining the header of a .texture file
typedef struct {
	int header_size; // Number of bytes in header data of file (not sizeof(struct))
	int file_size; // Number of bytes in data chunk of file (everything after the header + name + mip = .tga file or .dds file)
	int width, height;
	TexOptFlags flags;
	F32 fade[2];
	U8 alpha; // boolean flag
	U8 pad[3]; // =TEX for v1 .Textures, =TX2 for v2 .Textures
} TextureFileHeader;
// Next comes the name (sz)
// Next comes the mipmap data (header+data) to preload (ignored/not there in v1 .textures)

typedef struct {
	int structsize; // sizeof(TextureFileMipHeader)
	int width, height; // of first preload mip level
	int format; // Normally gathered from the .dds file header
} TextureFileMipHeader;


/*ddsLoad and tgaLoad functions take a TexReadInfo ptr and fill it out with the texture's data.
This is then used by texLoadImage when creating the OGL texture object.  The only place outside 
ddsLoad and tgaLoad TexReadInfo is changed is when ->data is changed by bumpMakeNormalMap and 
when ->data is freed when texLoadImage is done with it. 
Strange:
->pal seems to be used only byReadTGAColorMap and never gets alloced or freed, as far as I can tell.)
->depth is set to one in ReadTGAHeader, never seen again
->size seems to be used only internally to ddsLoad, but is used for tex mem usage profiling as well
some params might need to stay around cuz gettex uses them.
*/
typedef struct TexReadInfo
{
	U8		*data;			//texture data 
	int		mip_count;		//number of mip maps for the dds
	int		format;			//one of the flags below (TEXFMT_8BIT, TEXFMT_I_8, etc) or a flag defined in glext.h like GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 
							//used to set texbind->flags for compression level, to calculate block_size, and then as 
							//a flag to glCompressedTexImage2DARB.	

	int		width,height;	//texture height and width - stores the actual w/h after mip reduction, but isn't used
	int		size;			//bytes of texture data
} TexReadInfo;

enum
{
	TEXFMT_8BIT,
	TEXFMT_I_8,
	TEXFMT_16BIT,
	TEXFMT_ARGB_1555,
	TEXFMT_ARGB_0565,
	TEXFMT_ARGB_4444,
	TEXFMT_32BIT,
	TEXFMT_ARGB_8888,
	TEXFMT_ARGB_0888,
	TEXFMT_P_8,
	TEXFMT_RAW_DDS, // For loading raw data to be uncompressed in software
};

typedef enum TexFlags
{
	TEX_ALPHA		= 1 << 0,
	TEX_RGB8		= 1 << 1,
	TEX_COMP4		= 1 << 2,
	TEX_COMP8		= 1 << 3,
	
	TEX_TGA			= 1 << 5,
	TEX_DDS			= 1 << 6,


	TEX_CUBEMAPFACE	= 1 << 9,
	TEX_REPLACEABLE	= 1 << 10,
	TEX_BUMPMAP		= 1 << 11,

	TEX_JPEG		= 1 << 13,
} TexFlags;

typedef struct TexWord TexWord;

typedef struct TexWordParams TexWordParams;
typedef struct TexWordLoadInfo TexWordLoadInfo;
typedef struct TexOpt TexOpt;
typedef struct ScrollsScales ScrollsScales;

typedef struct BasicTexture
{
	const char	*name;

	struct BasicTexture *actualTexture; // This may be set to something different if the scene has swapped textures

	int		width;		// Logical width/height
	int		height;
	int		id;			// OpenGL ID

	TexFlags	flags;				//texture flags described below (TEX_ALPHA,TEX_RGB8, etc) (some set during texLoadData)

	TexOptFlags texopt_flags;
	U32		texopt_surface;

	U32		last_used_time_stamp[2];//0==regular, 1==raw
	TexLoadState load_state[2];		//has the data been loaded (are the id handles valid for it) 0==regular, 1==raw

	U32		file_pos;				//Offset of the texture data from file start
	U32		file_bytes;				//Size of texture data
	const char	*dirname;				//texture directory name ("WORLD/Filler/") - tack on texture_library/ and name and .texture to get the filename

	int		target;					//OpenGL target flag (GL_TEXTURE_CUBE_MAP_ARB or GL_TEXTURE_2D)
	F32		gloss;					//for bumpmapping

	// Preloaded mipmap data
	char	*mipdata;
	int		mipsize;
	int		mipid;

	TexReadInfo *rawInfo;
	int		rawReferenceCount;		// Count of texLoadRawData()s outstanding

	// Texture compositing
	TexWord *texWord;
	int		origWidth, origHeight;
	TexWordParams *texWordParams; // For dynamic textures
	TexWordLoadInfo *texWordLoadInfo;

	U32		hasBeenComposited:1;	// If the texture has a texWord*, has it been applied?

	TexUsage	use_category;	//what is this texture used for?
	int		memory_use[2];

	S32		cube_face_idx:8;		// Used by textures that are cube maps 

	int		realWidth, realHeight;	// GL size of the texture, power of 2

} BasicTexture;

typedef struct TexBind
{
	const char	*name;

	int		width;	// Width and height of base texture (used for sprites/2D)
	int		height;

	const char	*dirname;				// Either directory name of the texture, or trick file name if this is a trick-made texture

	BlendModeType	bind_blend_mode;//pixel shader to use (BLENDMODE_MULTIPLY etc)
	F32		bind_scale[2][2];		//st scale for this tex and secondary texture, if any (tex_links[0])

	TexUsage	use_category;	//what is this texture used for?

	BasicTexture *tex_layers[TEXLAYER_MAX_LAYERS];	//set in texsetbinds, given ids in texConvertToGameFormat
	U8		tex_swappable[TEXLAYER_MAX_LAYERS];	// Whether or not, on a MultiTexture shader, each layer can be swapped
	U8		needs_alphasort:1;
	U8		allocated_scrollsScales:1; // We allocated our own scrollsScales struct that needs to be freed
	U8		allocated_byMiniTracker:1;
	U8		is_fallback_material:1;
	TexOpt	*texopt;
	ScrollsScales *scrollsScales;
	TexBind *tex_lod; // generated on the fly when needed by the LOD system
	TexBind *orig_bind; // points to self unless this is an lod of a tex bind
} TexBind;

typedef struct TexThreadPackage
{
	struct TexThreadPackage * next;
	struct TexThreadPackage * prev;

	BasicTexture *bind;		// bind being worked on
	TexReadInfo info;
	int should_queue; // set to -1 if it should be based on the current state of the queuing variable in the thread
	int needRawData;
} TexThreadPackage;


extern TexBind *white_tex_bind, *invisible_tex_bind, *grey_tex_bind, *black_tex_bind; 
extern BasicTexture *white_tex, *invisible_tex, *grey_tex, *black_tex, *dummy_bump_tex, *dummy_dynamic_cubemap_tex, *building_lights_tex;
extern int delay_texture_loading;  // Adds a delay to simulate slow loading

extern int texMemoryUsage[2];

extern BasicTexture **g_basicTextures;  // EArray of binds
extern TexBind **g_compositeTextures;  // EArray of binds

void texLoadQueueStart(void); // Directs the loader to queue successive load calls
void texLoadQueueFinish(void); // Directs the loader to stop queueing load calls and begin loading all queued graphics
void texLoadQueueDefer(void); // Directs the loader to stop queueing load calls, but not begin loading them
void texLoadQueueResume(void); // Stops deferring, and begins queuing again

void texResetBinds(void);
void texResetAnisotropic(void);
int texLoadHeaders(bool bForceTexCache);
void texFree( BasicTexture * bind, int freeRawData );
bool texRemoveRefBasic( BasicTexture *basicBind ); // For TexWordsEditor
void texRemoveRef( TexBind* bind ); // For dynamic textures, used to say a GroupDef is down with it
BasicTexture *texFind(const char *name);
BasicTexture *texFindRandom(void);
TexBind *texFindComposite(const char *name);
void texCreateLOD(TexBind *orig_bind);
void texFreeLOD(TexBind *orig_bid);
void texCheckThreadLoader(void);
TexBind *texLoad(const char *name, TexLoadHow mode, TexUsage use_purpose);
BasicTexture *texLoadBasic(const char *name, TexLoadHow mode, TexUsage use_category);
BasicTexture *texLoadRawData(const char *name, TexLoadHow mode, TexUsage use_category);
void texUnloadRawData(BasicTexture *texBind);
TexBind *texLoadDynamic(TexWordParams *params, TexLoadHow mode, TexUsage use_purpose, const char *blameFileName);
TexBind *texCreateTrickTexture(const char *dirname, const char *name, const char *basetexture);
void texSetBindsSubComposite(TexBind *bind);
void texFreeAll(void);
void texMakeWhite(void);
void texShowUsage(char *match_string);
void texShowUsageRes(int xres, int yres);
void texCheckSwapList(void);
void texGatherStatistics(void);
long texLoadsPending(int include_texwords); // Returns the number of texture loads that have been sent to the background loader that have not finished yet
void texSetSearchPath(char *searchPath); // Expects something like "#English", "#French;#Base" or just ""
int texGetNumMipsToSkip(U32 tex_width, U32 tex_height, int is_for_entity);

void texDisableThreadedLoading(void);
void texEnableThreadedLoading(void);
int texFindByFullPathName(char *search,const char **names,int max);
void texForceTexLoaderToComplete(int force_texword_flush);

int texDemandLoadActual(BasicTexture *texbind);
int texGetFormat(BasicTexture *bind);
void rdrTexFree(int id);
typedef struct RdrTexParams RdrTexParams;
void rdrTexCopy(RdrTexParams *rtex,void *data,int num_bytes);
typedef struct RdrSubTexParams RdrSubTexParams;
void rdrTexSubCopy(RdrSubTexParams *rtex,void *data,int num_bytes);
void texSetupParamsFromBind(BasicTexture *tex_bind, TexReadInfo *info,RdrTexParams *rtex);
void texScanForTexHeaders(bool bInitialLoad, bool bForceTexCache);
int texGetNumBasicTextures(void);

int texReadFile(char * texName, char * directoryName, TexReadInfo * info);
int texWriteFile(char * texName, TexReadInfo * info);
static INLINEDBG int texDemandLoad(BasicTexture *texbind)
{
	BasicTexture *actualTexture;
	extern int client_frame_timestamp;

	if (!texbind)
		return 0;
	actualTexture = texbind->actualTexture;

	if (!actualTexture)
		return 0;

	actualTexture->last_used_time_stamp[0] = client_frame_timestamp;
	if (actualTexture->load_state[0] == TEX_LOADED)
		return actualTexture->id;
	return texDemandLoadActual(texbind);
}

static INLINEDBG int texDemandLoadComposite(TexBind *texbind)
{
	extern int client_frame_timestamp;
	return texDemandLoad(texbind->tex_layers[TEXLAYER_BASE1]);
}

static INLINEDBG TexBind *texFindLOD(TexBind *orig_bind)
{
	if (!orig_bind->tex_lod)
		texCreateLOD(orig_bind);
	return orig_bind->tex_lod;
}


#endif
