#ifndef _RENDERLOOP_H
#define _RENDERLOOP_H

#include "stdtypes.h"
#include "WorkerThread.h"
#include "failtext.h"

extern WorkerThread *render_thread;

typedef enum
{
	DRAWCMD_RDRSETUP = WT_CMD_USER_START,

	// Basic stuff
	DRAWCMD_CLEAR,
	DRAWCMD_PERSPECTIVE,
	DRAWCMD_VIEWPORT,
	DRAWCMD_SWAPBUFFER,
	DRAWCMD_INITDEFAULTS,
	DRAWCMD_FOG,
	DRAWCMD_BGCOLOR,

	// get data from gl
	DRAWCMD_GETPROJECTIONMATRIX,
	DRAWCMD_GETFRAMEBUFFER,
	DRAWCMD_GETTEXINFO,

	// texture loading
	DRAWCMD_TEXFREE,
	DRAWCMD_TEXCOPY,
	DRAWCMD_TEXSUBCOPY,
	DRAWCMD_GENTEXHANDLES,

	// buffers
	DRAWCMD_GENBUFFERHANDLES,
	DRAWCMD_FREEBUFFER,
	DRAWCMD_FREEMEM,

	// Sprite drawing
	DRAWCMD_SETUP2D,
	DRAWCMD_FINISH2D,
	DRAWCMD_SPRITE,
	DRAWCMD_SPRITES,
	DRAWCMD_SIMPLETEXT,
	DRAWCMD_DYNAMICQUAD,	// dynamic quad paint (e.g. movie frame, webkit paint)

	// lines and boxes
	DRAWCMD_FILLEDBOX,
	DRAWCMD_LINE2D,
	DRAWCMD_LINE3D,
	DRAWCMD_FILLEDTRI,

	// Pbuffer
	DRAWCMD_PBUFMAKECURRENT,
	DRAWCMD_WINMAKECURRENT,
	DRAWCMD_PBUFINIT,
	DRAWCMD_PBUFDELETE,
	DRAWCMD_PBUFDELETEALL,
	DRAWCMD_PBUFBIND,
	DRAWCMD_PBUFRELEASE,
	DRAWCMD_PBUFSETUPCOPYTEXTURE,

	// Cursor
	DRAWCMD_CURSORCLEAR,
	DRAWCMD_CURSORBLIT,

	// 3D models
	DRAWCMD_MODEL,
	DRAWCMD_MODELALPHASORTHACK,
	DRAWCMD_BONEDMODEL,
	DRAWCMD_CONVERTRGBBUFFER,
	DRAWCMD_CREATEVBO,
	DRAWCMD_FREEVBO,
	DRAWCMD_RESETVBOS,
	DRAWCMD_PERFTEST,
	DRAWCMD_PERFSET,

	// shadows
	DRAWCMD_SHADOWFINISHFRAME,
	DRAWCMD_STENCILSHADOW,
	DRAWCMD_SPLAT,
	DRAWCMD_STENCIL,
	
	// viewport (generic, specific ports may have their own custom)
	DRAWCMD_VIEWPORT_PRE,
	DRAWCMD_VIEWPORT_POST,

	// shadowmaps
	DRAWCMD_INITSHADOWMAPMENU,
	DRAWCMD_SHADOWMAP_RENDER,
	DRAWCMD_SHADOWVIEWPORT_PRECALLBACK,
	DRAWCMD_SHADOWVIEWPORT_POSTCALLBACK,
	DRAWCMD_REFRESHSHADOWMAPS,
	DRAWCMD_SHADOWMAP_BEGIN_USING,
	DRAWCMD_SHADOWMAP_FINISH_USING,
	DRAWCMD_SHADOWMAPDEBUG_2D,
	DRAWCMD_SHADOWMAPDEBUG_3D,
	DRAWCMD_SHADOWMAP_SCENE_SPECS,
	
	// cubemaps
	DRAWCMD_INITCUBEMAPMENU,
	DRAWCMD_CUBEMAP_PRECALLBACK,
	DRAWCMD_CUBEMAP_POSTCALLBACK,
	DRAWCMD_CUBEMAP_SETINVCAMMATRIX,
	DRAWCMD_CUBEMAP_SETSET3FACEMATRIX,

	// Clip Planes
	DRAWCMD_CLIPPLANE,
	
	// Particles
	DRAWCMD_PARTICLESETUP,
	DRAWCMD_PARTICLES,
	DRAWCMD_PARTICLEFINISH,

	// Gamy
	DRAWCMD_INIT,
	DRAWCMD_INITTOPOFFRAME,
	DRAWCMD_INITTOPOFVIEW,
	DRAWCMD_RELOADSHADER,
	DRAWCMD_REBUILDCACHEDSHADERS,
	DRAWCMD_SHADERDEBUGCONTROL,
	DRAWCMD_SETCGSHADERMODE,
	DRAWCMD_SETNORMALMAPTESTMODE,
	DRAWCMD_SETDXT5NMNORMALS,
	DRAWCMD_SETCGPATH,
	DRAWCMD_CLOTH,
	DRAWCMD_CLOTHFREE,
	DRAWCMD_SETANISO,
	DRAWCMD_RESETANISO,
	DRAWCMD_RESETANISO_CUBEMAP,
	DRAWCMD_WRITEDEPTHONLY,

	// Water
	DRAWCMD_WATERSETPARAMS,
	DRAWCMD_FRAMEGRAB,
	DRAWCMD_SETREFLECTIONPLANE,

	// Ambient
	DRAWCMD_SSAOSETPARAMS,
	DRAWCMD_SSAORENDER,
	DRAWCMD_SSAODEBUGINPUT,
	DRAWCMD_SSAORENDERDEBUG,

	// Effects
	DRAWCMD_POSTPROCESSING,
	DRAWCMD_HDRDEBUG,
	DRAWCMD_SUNFLAREUPDATE,
	DRAWCMD_RENDERSCALED,
	DRAWCMD_ADDTEXTURES,
	DRAWCMD_MULTEXTURES,
	DRAWCMD_ADDITIVE_ALPHA_WRITE_MASK,
	DRAWCMD_SETUP_STIPPLE,
	DRAWCMD_OUTLINE,

	// Debug
	DRAWCMD_FRAMEGRAB_SHOW,
	DRAWCMD_DEBUGCOMMANDS,

	// Render stats
    DRAWCMD_FRAMESTART,
	DRAWCMD_RESETSTATS,
	DRAWCMD_GETSTATS,

	// Markers
	DRAWCMD_BEGINMARKERFRAME,
	DRAWCMD_BEGINMARKER,
	DRAWCMD_ENDMARKER,
	DRAWCMD_SETMARKER,

	// win init
	DRAWCMD_SETGAMMA,
	DRAWCMD_NEEDGAMMARESET,
	DRAWCMD_RESTOREGAMMA,
	DRAWCMD_DESTROYDISPLAYCONTEXTS,
	DRAWCMD_RELEASEDISPLAYCONTEXTS,
	DRAWCMD_REINITDISPLAYCONTEXTS,
	DRAWCMD_SETVSYNC,

} DrawType;

typedef struct
{
	Vec2	fog_dist;
	Vec3	fog_color;
	U32		has_color : 1;
} RdrFog;

typedef struct
{
	Vec3	points[3];
	U32		colors[4];
	F32		line_width;
} RdrLineBox;

typedef enum
{
	GETFRAMEBUF_RGB = 1,
	GETFRAMEBUF_RGBA,
	GETFRAMEBUF_DEPTH,
	GETFRAMEBUF_STENCIL,
	GETFRAMEBUF_DEPTH_FLOAT,
} GetFrameBufType;

typedef struct
{
	int				x,y;
	int				width,height;
	void			*data;
	GetFrameBufType	type;
} RdrGetFrameBuffer;

typedef struct
{
	int				texid;
	int				*widthout, *heightout;
	int				pixeltype;
	int				pixelformat;
	void			*data;
} RdrGetTexInfo;

typedef struct
{
	U32				numplanes : 3;
	F32				planes[6][4];
} RdrClipPlane;

typedef enum
{
	STENCILOP_KEEP = 1,
	STENCILOP_INCR,
} RdrStencilOp;

typedef struct
{
	U32				enable : 1;
	RdrStencilOp	op;
	U32				func_ref;
} RdrStencil;

typedef struct
{
	int x, y;
	int width,height;
	bool bNeedScissoring;
} Viewport;

typedef struct
{
	F32 fovy,aspect,znear,zfar;

	// flags only used for ortho mode
	U32	ortho : 1;
	int	width,height;
	int ortho_zoom;
} Perspective;

typedef struct	/*used by DRAWCMD_DYNAMICQUAD*/

{
	int				dirty;					// texture needs to be updated
	int				width_src, height_src;
	int				format;
	void			*pixels;
	int				top_dst, left_dst, bottom_dst, right_dst;	// screen space destination
} RdrDynamicQuad;

typedef enum ClearScreenFlags {
	CLEAR_ALPHA = 1<<0,
	CLEAR_DEPTH = 1<<1,
	CLEAR_STENCIL = 1<<2,
	CLEAR_COLOR = 1<<3,
	CLEAR_DEPTH_TO_ZERO = 1<<4,

	CLEAR_DEFAULT = CLEAR_DEPTH|CLEAR_STENCIL|CLEAR_COLOR
} ClearScreenFlags;

typedef struct RdrAddTexturesParams RdrAddTexturesParams;
typedef struct RdrShrinkTextureParams RdrShrinkTextureParams;

void rdrAddTextures(RdrAddTexturesParams *params);
void rdrMulTextures(RdrAddTexturesParams *params);
void rdrSetWriteDepthOnly(int write_depth_only);

int rdrAllocTexHandle(void);
int rdrAllocBufferHandle(void);

void renderThreadSetThreading(int run_thread);
void renderThreadStart(void);

static INLINEDBG void rdrQueue(DrawType type,const void *data,int size) { wtQueueCmd(render_thread,type,data,size); }
static INLINEDBG void rdrQueueCmd(DrawType type) { wtQueueCmd(render_thread,type,0,0); }
static INLINEDBG void rdrQueueFlush(void) { failText("Render queue flushed (performance warning)"); wtFlush(render_thread); }
static INLINEDBG void rdrQueueMonitor(void) { wtMonitor(render_thread); }
static INLINEDBG int rdrIsThreaded(void) { return wtIsThreaded(render_thread); }
static INLINEDBG void rdrQueueDebug(WTDebugCallback callback_func,void *data,int size) { wtQueueDebugCmd(render_thread,callback_func,data,size); }
static INLINEDBG void *rdrQueueAlloc(DrawType type,int size) { return wtAllocCmd(render_thread,type,size); }
static INLINEDBG void rdrQueueCancel(void) { wtCancelCmd(render_thread); }
static INLINEDBG void rdrQueueSend(void) { wtSendCmd(render_thread); }

static INLINEDBG bool rdrInThread(void) { return wtInWorkerThread(render_thread); }
static INLINEDBG void rdrCheckThread(void) { wtAssertWorkerThread(render_thread); }
static INLINEDBG void rdrCheckNotThread(void) { wtAssertNotWorkerThread(render_thread); }

#endif
