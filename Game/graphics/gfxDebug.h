#include "stdtypes.h"
#include "rt_state.h"

typedef struct GPolySet GPolySet;

void gfxDump360(void);
void gfxShaderPerfTest(void);
void gfxScreenDump(char *subdir, char *legacysubdir, int is_movie, char * format_extension);
void gfxScreenDumpEx(char *subdir, char *legacysubdir, int is_movie, char *extra_jpeg_data, int extra_jpeg_data_len, char * format_extension);
void addDebugLine( Vec3 va, Vec3 vb, U8 r, U8 b, U8 g, U8 a, F32 width );
void gfxShowDebugIndicators(void); // Happens before processing everything in a frame
void gfxShowDebugIndicators2(void); // Happens after processing everything in a frame
void gfxSetDebugTexture(int handle);
void gfxDrawGPolySet(GPolySet *set);
void gfxRenderDumpColor( char *fn, ...);
void gfxDebugUpdate(void);
void gfxDebugInput(void);

extern int setBlendStateCalls[BLENDMODE_NUMENTRIES];
extern int setBlendStateChanges[BLENDMODE_NUMENTRIES];
extern int setDrawStateCalls[DRAWMODE_NUMENTRIES];
extern int setDrawStateChanges[DRAWMODE_NUMENTRIES];

enum GFXDEBUG_FLAGS {
	// Normal and Tangent Basis (TBN) display options
	GFXDEBUG_NORMALS				= 1<<0,
	GFXDEBUG_BINORMALS				= 1<<1,
	GFXDEBUG_TANGENTS				= 1<<2,
	GFXDEBUG_TBN_SKIP_LEFT_HANDED	= 1<<3,	// skip drawing TBN that are left handed (the default for CoH normal maps)
	GFXDEBUG_TBN_SKIP_RIGHT_HANDED	= 1<<4,	// skip drawing TBN that are right handed (e.g., from mirrored UVs)
	GFXDEBUG_TBN_FLIP_HANDEDNESS	= 1<<5, // flip the TBN basis handedness on model load

	GFXDEBUG_SEAMS					= 1<<8,
	GFXDEBUG_VERTINFO				= 1<<9,
	GFXDEBUG_LIGHTDIRS				= 1<<10,

	GFXDEBUG_SKIP_CHECKGLSTATE		= 1<<16, // skip WCW_CheckGLState() which makes for easier to inspect GL marker trace captures
};

// Flags used to control scene graph traversal and rendering for debug and eval purposes
// These are the flags set with the '/perf' console command and tweak menu
enum GFXDEBUG_PERF_FLAGS {
	// Normal and Tangent Basis (TBN) display options
	GFXDEBUG_PERF_SKIP_2D						= 1<<0,
	GFXDEBUG_PERF_FORCE_SORT_BY_TYPE			= 1<<1,
	GFXDEBUG_PERF_FORCE_SORT_BY_DIST			= 1<<2,
	GFXDEBUG_PERF_SKIP_SORT_BY_TYPE				= 1<<3,
	GFXDEBUG_PERF_SKIP_SORT_BY_DIST				= 1<<4,
	GFXDEBUG_PERF_SKIP_SKY						= 1<<5,
	GFXDEBUG_PERF_SKIP_GROUPDEF_TRAVERSAL		= 1<<6,	// environment geo
	GFXDEBUG_PERF_SKIP_GFXNODE_TREE_TRAVERSAL	= 1<<7, // entity geo
	GFXDEBUG_PERF_SKIP_OPAQUE_PASS				= 1<<8,
	GFXDEBUG_PERF_SKIP_SORTING					= 1<<9,
	GFXDEBUG_PERF_SKIP_ALPHA_PASS				= 1<<10,
	GFXDEBUG_PERF_SKIP_UNDERWATER_ALPHA_PASS	= 1<<11,
	GFXDEBUG_PERF_SORT_BY_REVERSE_DIST			= 1<<12,
	GFXDEBUG_PERF_SORT_RANDOMLY					= 1<<13,
	GFXDEBUG_PERF_SKIP_LINES					= 1<<14,
};

#ifndef FINAL
#define GFX_DEBUG_TEST( val, flag )	(((val)&(flag))!=0)
#else
#define GFX_DEBUG_TEST( val, flag )	(0)
#endif