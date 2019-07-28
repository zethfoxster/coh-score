#include <stdio.h>
#include "ogl.h"
#include "pbuffer.h"
#include "error.h"
#include "win_init.h"
#include "renderUtil.h"
#include "wcw_statemgmt.h"
#include "timing.h"
#include "rendertree.h"
#include "camera.h"
#include "font.h"
#include "sprite_text.h"
#include "tex.h"
#include "Color.h"
#include "renderprim.h"
#include "cmdgame.h"

void winMakeCurrent()
{
	extern int override_window_size;
	override_window_size = 0;
	rdrQueueCmd(DRAWCMD_WINMAKECURRENT);
}

void pbufMakeCurrent(PBuffer *pbuf)
{
	{
		extern int override_window_size, override_window_size_width, override_window_size_height;
		override_window_size = 1;
		override_window_size_width = pbuf->width;
		override_window_size_height = pbuf->height;
	}
	rdrQueue(DRAWCMD_PBUFMAKECURRENT,&pbuf,sizeof(pbuf));
}

void pbufDelete(PBuffer *pbuf)
{
	rdrQueue(DRAWCMD_PBUFDELETE,&pbuf,sizeof(pbuf));
	rdrQueueFlush();
}

void pbufDeleteAll(void)
{
	rdrQueueCmd(DRAWCMD_PBUFDELETEALL);
	rdrQueueFlush();
}


void pbufInit(PBuffer *pbuf,int w,int h,PBufferFlags flags,int desired_multisample_level, int required_multisample_level, int stencil_bits, int depth_bits, int num_aux_buffers, int max_mip_level)
{
	RdrPbufParams	params;

	CHECKNOTGLTHREAD;

	// Make sure we don't try to use FBOs when we should not.
	if (!rdr_caps.supports_fbos)
		flags &= ~PB_FBO;
		
	// If we do not need to recreate the pbuffer, don't!
	if (pbuf->width == w && pbuf->height == h &&
		desired_multisample_level == pbuf->desired_multisample_level &&
		num_aux_buffers == pbuf->num_aux_buffers &&
		pbuf->flags == flags)
	{
		// Not a perfect test for the stencil, but should be good enough
		// TODO: Also catch depth and mip changes
		if ((stencil_bits != 0) == ((bool) pbuf->hasStencil))
		{
			return;
		}

		// TODO: Handle non-FBO case?
	}

	params.pbuf							= pbuf;
	params.width						= w;
	params.height						= h;
	params.flags						= flags;
	params.desired_multisample_level	= desired_multisample_level;
	params.required_multisample_level	= required_multisample_level;
	params.stencil_bits					= stencil_bits;
	params.depth_bits					= depth_bits;
	params.num_aux_buffers				= num_aux_buffers;
	params.max_mip_level				= max_mip_level;
	rdrQueue(DRAWCMD_PBUFINIT,&params,sizeof(params));
	rdrQueueFlush();
	rdrStippleStencilNeedsRebuild();
}

U8 *pbufFrameGrab(PBuffer *pbuf, U8* buffer, U32 buffer_size)
{
	U8		*pixbuf;
	U32		size;

	CHECKNOTGLTHREAD;
	PERFINFO_AUTO_START("pbufFrameGrab", 1);
		pbufMakeCurrent(pbuf);
		size = pbuf->width * pbuf->height * 4;
		if(size > buffer_size)
			pixbuf = malloc(size);
		else
			pixbuf = buffer;

		PERFINFO_AUTO_START("glReadPixels", 1);
		rdrGetFrameBuffer(0,0,pbuf->width,pbuf->height, GETFRAMEBUF_RGBA, pixbuf);
		PERFINFO_AUTO_STOP();
		PERFINFO_AUTO_START("DownSample", 1);
		{
			int w, h, neww, newh;
			int multisample = pbuf->software_multisample_level;
			w = pbuf->width;
			h = pbuf->height;
			while (multisample>1) {
				// Downsample in-place
				int	i,x,y,y_in[2];
				U32	pixel,rgba[4],*pix;

				neww = w >> 1;
				newh = h >> 1;

				for(pix=(U32*)pixbuf,y=0;y<newh;y++)
				{
					y_in[0] = (y*2+0) * w;
					y_in[1] = (y*2+1) * w;
					for(x=0;x<neww;x++)
					{
						memset(rgba, 0, sizeof(rgba));
						for(i=0;i<4;i++)
						{
							pixel = ((U32*)pixbuf)[y_in[i>>1] + (x<<1) + (i&1)];
							rgba[0] += (pixel >> (8 * 0)) & 255;
							rgba[1] += (pixel >> (8 * 1)) & 255;
							rgba[2] += (pixel >> (8 * 2)) & 255;
							rgba[3] += (pixel >> (8 * 3)) & 255;
						}
						*pix++ = (rgba[0] >> 2) << (8 * 0)
							| (rgba[1] >> 2) << (8 * 1)
							| (rgba[2] >> 2) << (8 * 2)
							| (rgba[3] >> 2) << (8 * 3);
					}
				}

				w = neww;
				h = newh;
				multisample >>= 1;
			}
		}
		if (!rdr_caps.supports_pbuffers)
		{
			// We aren't really using a PBuffer here, so try to fake Alpha
			U32	*pix;
			int size = pbuf->width*pbuf->height;
			int i;
			Color c;
			for(pix=(U32*)pixbuf, i=0; i<size; i++, pix++)
			{
				c.integer = *pix;
				if (c.r+c.g+c.b==0)
					c.a=0;
				*pix = c.integer;
			}
		}
		PERFINFO_AUTO_STOP();
		winMakeCurrent();
	PERFINFO_AUTO_STOP();
	rdrQueueFlush();
	return pixbuf;
}

void pbufBind(PBuffer *pbuf) // Binds as a texture
{
	rdrQueue(DRAWCMD_PBUFBIND,&pbuf,sizeof(pbuf));
}

void pbufRelease(PBuffer *pbuf) // Releases as a texture
{
	rdrQueue(DRAWCMD_PBUFRELEASE,&pbuf,sizeof(pbuf));
}

bool pbufStencilSupported(PBufferFlags flags)
{
	if ((flags & PB_FBO) && !rdr_caps.supports_packed_stencil)
		return false;

	return true;
}

void pbufSetupCopyTexture(PBuffer *pbuf, int texture_id)
{
	RdrPbufSetCopyTextureParams params;

	assert(pbuf);
	assert(texture_id > -1);

	params.pbuf = pbuf;
	params.copy_texture_id = texture_id;

	rdrQueue(DRAWCMD_PBUFSETUPCOPYTEXTURE,&params,sizeof(params));
}


