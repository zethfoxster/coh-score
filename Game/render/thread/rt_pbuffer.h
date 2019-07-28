#ifndef _RT_PBUFFER_H
#define _RT_PBUFFER_H

#include "stdtypes.h"

#define MAX_SLI 4U
#define MAX_FRAMES (MAX_SLI + 2U)

typedef struct FogContext FogContext;

typedef enum PBufferFlags {
	PB_RENDER_TEXTURE		= 1<<0,
	PB_FLOAT				= 1<<1,
	PB_NOSHARE				= 1<<2,		// Share is default
	PB_FBO					= 1<<3,
	PB_COLOR_TEXTURE		= 1<<4,		// If not set, then for FBOs, only a render buffer is created
	PB_DEPTH_TEXTURE		= 1<<5,
	PB_ALPHA				= 1<<6,
	PB_ALLOW_FAKE_PBUFFER	= 1<<7,		// Allows it to just use a corner of the backbuffer
	PB_CUBEMAP				= 1<<8, 
	PB_MIPMAPPED			= 1<<9,
	PB_ONESHOTCOPY			= 1<<10,
	PB_COLOR_TEXTURE_AUX	= 1<<11,
} PBufferFlags;

typedef struct PBuffer PBuffer;

typedef struct PBuffer
{
	PBuffer		*next;
	PBuffer		*prev;
    void		*myDC;      // Handle to a device context.
    void		*myGLctx;   // Handle to a GL context.
    void		*buffer;    // Handle to a pbuffer. // HPBUFFERARB
    PBufferFlags	flags;      // Flags indicating the type of pbuffer.
    int         width;
    int         height;
	int			virtual_width;
	int			virtual_height;
	int			depth;
	int			color_internal_format;
	int			color_format;
	int			color_type;
	int			depth_internal_format;
	int			depth_format;
	int			depth_type;
	int			hardware_multisample_level;
	int			software_multisample_level;
	int			desired_multisample_level;
	int			required_multisample_level;
	int			allocated_fbos;
	int			color_handle[MAX_SLI];	// GL Texture handle if using as render to texture
	int			color_rbhandle[MAX_SLI];// GL RenderBuffer handle to color render buffer if FBO MSAA
	int			depth_handle[MAX_SLI];	// GL Texture handle to depth texture if FBO or rendering depth to texture
	int			depth_rbhandle[MAX_SLI];// GL RenderBuffer handle to depth render buffer if FBO
	//int			stencil_rbhandle;		// GL RenderBuffer handle to stencil
	int			aux_handles[4];			// GL RenderBuffer handle for aux buffers if using FBOs (@todo legacy MRT support, needs to be updated to be used)
	int			fbo[MAX_SLI];			// GL FBO handle
	int			blit_fbo[MAX_SLI];		// GL FBO handle for blitting from color/depth renderbuffer to color/depth texture
	int			blit_flags;				// Flags for blitting
	int			num_aux_buffers;		// How many AUX buffers (@todo legacy MRT support, needs to be updated to be used)
	int			currentCubemapTarget;	// GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, etc.  Only set for PB_CUBEMAP
	U32         curFrame;               // Last frame this buffer was used
	U8          curFBO;                 // Current FBO index when using SLI FBOs
	U32			isDirty:1;				// If a "pretend" RTTPB, this gets flagged if the PBuffer needs to be recopied
	U32			isBound:1;
	U32			isFakedPBuffer:1;		// Pretend PBuffer (just rendering to corner of back buffer)
	U32			isFakedRTT:1;			// Pretend RTT (copying texture manually)
	U32			isFloat:1;				// Floating point buffer
	U32			hasStencil:1;			// stencil bits packed into depth buffer
	FogContext	*fog_context;
	int			copy_texture_id;
	int			color_handle_aux[MAX_SLI];// aux texture attachment used for optional filtering (@todo legacy MRT num_aux_buffers, aux_buffers could be brought up to date and repurposed)	
} PBuffer;

typedef struct RdrPbufParams
{
	PBuffer *pbuf;
	int		width,height;
	PBufferFlags	flags;
	int		desired_multisample_level;
	int		required_multisample_level;
	int		stencil_bits;
	int		depth_bits;
	int		num_aux_buffers;
	int		max_mip_level;		// ignored if PB_MIPMAPPED is not specified
	int		copy_texture_id;	// ignored if PB_ONESHOTCOPY is not specified
} RdrPbufParams;

typedef struct RdrPbufSetCopyTextureParams
{
	PBuffer *pbuf;
	int		copy_texture_id;
} RdrPbufSetCopyTextureParams;

#ifdef RT_PRIVATE
void pbufInitDirect(RdrPbufParams *params);
void pbufInitDirect2(PBuffer *pbuf,int w,int h,PBufferFlags flags,int desired_multisample_level, int required_multisample_level, int stencil_bits, int depth_bits, int num_aux_buffers);
void pbufDeleteDirect(PBuffer **pbuf_ptr);
void pbufDeleteAllDirect(void);
void pbufMakeCurrentDirect(PBuffer *pbuf);
void pbufBindDirect(PBuffer *pbuf);
void pbufBindDirect2(PBuffer *pbuf, int tex_unit);
void pbufBindAuxDirect(PBuffer *pbuf, int tex_unit, int buffer);
void pbufBindDepthDirect(PBuffer *pbuf, int tex_unit);
void pbufReleaseDirect(PBuffer *pbuf);
void pbufReleaseDirect2(PBuffer *pbuf, int tex_unit);
void pbufReleaseAuxDirect(PBuffer *pbuf, int tex_unit, int buffer);
void pbufReleaseDepthDirect(PBuffer *pbuf, int tex_unit);
PBuffer *pbufGetCurrentDirect(void);
void winMakeCurrentDirect(void);

void pbufSetCopyTextureDirect(RdrPbufSetCopyTextureParams *params);

void winCopyFrameBufferDirect(int tex_target, int rgba_texture_handle, int depth_texture_handle, int x, int y, int width, int height, int miplevel, bool flipX, bool flipY);
void pbufCopyDirect(PBuffer *pbuf, int tex_target, int rgba_texture_handle, int depth_texture_handle, int x, int y, int width, int height);
#endif

#endif
