#ifndef _RT_TEX_H
#define _RT_TEX_H

#include "texEnums.h"
#include "rt_queue.h"

typedef struct RdrTexParams
{
	U16		width,height,x,y;
	U32		clamp_s : 1;
	U32		clamp_t : 1;
	U32		mirror_s : 1;
	U32		mirror_t : 1;
	U32		mipmap : 1;
	U32		point_sample : 1;
	U32		dxtc : 1;
	U32		no_file_corruption_check : 1;
	U32		mip_count : 4;
	U32		from_screen : 1;
	U32		sub_image : 1;
	U32		floatingpoint : 1;
	U32		tex_border : 1;
	int		id;
	int		target;
	int		src_format;
	int		dst_format;
} RdrTexParams;

typedef struct RdrSubTexParams
{
	U16		x_offset, y_offset;
	U16		width,height;
	U32		dxtc : 1;
	int		id;
	int		target;
	int		format;
} RdrSubTexParams;

#ifdef RT_PRIVATE
void texCopyDirect(RdrTexParams *rtex);
void texSubCopyDirect(RdrSubTexParams *rtex);
void texBindTexCoordPointerEx(TexLayerIndex texindex, int stride, const void *pointer);
static INLINEDBG void texBindTexCoordPointer(TexLayerIndex texindex, const void *pointer) { texBindTexCoordPointerEx(texindex, 0, pointer); }
void texEnableTexCoordPointer(TexLayerIndex texindex, bool enable);
void texSetWhite(TexLayerIndex texindex);
void texBind(TexLayerIndex texindex, int tex_id);
void texBindCube(TexLayerIndex texindex, int tex_id);
void rdrGetTexInfoDirect(RdrGetTexInfo *get);
void texSetAnisotropicDirect(int value);
void texResetAnisotropicDirect(int texid, bool bCubemap);
void rdrTexFreeDirect(int texid);

// Internal stuff (used by shadersTexEnv)
extern int				boundTextures[];
extern bool				texCoordPointersEnabled[];
extern const F32		*texCoordPointers[];
extern int				texCoordStrides[];
// End Internal
#endif

#endif
