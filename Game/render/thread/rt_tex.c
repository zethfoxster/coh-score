#define RT_PRIVATE
#define RT_ALLOW_BINDTEXTURE
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "error.h"
#include "renderprim.h"
#include "mipmap.h"
#include "tex.h"
#include "rt_tex.h"
#include "rt_state.h"
#include "SuperAssert.h"
#include "rt_pbuffer.h"

int			boundTextures[MAX_TEXTURE_UNITS_TOTAL];
bool		texCoordPointersEnabled[MAX_TEXTURE_UNITS_TOTAL];
const F32	*texCoordPointers[MAX_TEXTURE_UNITS_TOTAL];
int			texCoordStrides[MAX_TEXTURE_UNITS_TOTAL];
bool		g_renderstate_dirty = true;
int			texBindCalls;

#include "cmdgame.h"

void texCopyDirect(RdrTexParams *rtex)
{
	int		target = rtex->target, bindTarget;
	U32		i,width = rtex->width,height=rtex->height;
	U8		*bitmap = (U8*)(rtex+1);
	bool	bDontSetFlags;

	rdrBeginMarker(__FUNCTION__);
	CHECKGL;

	if (!target)
		target = GL_TEXTURE_2D;

	// special handling for cubemap
	if( target == GL_TEXTURE_CUBE_MAP_ARB )
	{
		// this is first face of cubemap
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB;
	}

	if( target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB)
		bindTarget = GL_TEXTURE_CUBE_MAP_ARB;
	else
		bindTarget = target;

	// only set flags for a bindTarget once, dont repeat for sub_image or subsequent cubemap faces
	bDontSetFlags = ( rtex->sub_image || (target >= GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB) );

	assert(bindTarget == GL_TEXTURE_2D || bindTarget == GL_TEXTURE_CUBE_MAP_ARB); // sanity check
	WCW_BindTexture(bindTarget, 0, rtex->id);

	if (!bDontSetFlags) {
		// setup texture flags
		if (rtex->tex_border) {
			glTexParameterf(bindTarget, GL_TEXTURE_WRAP_S, GL_CLAMP); CHECKGL;
			glTexParameterf(bindTarget, GL_TEXTURE_WRAP_T, GL_CLAMP); CHECKGL;
			rtex->tex_border = 0; // This doesn't seem to actually work, perhaps because of compressed textures?
		} else {
			if (rtex->clamp_s) {
				glTexParameterf(bindTarget, GL_TEXTURE_WRAP_S, gl_clamp_val); CHECKGL;
			}
			if (rtex->clamp_t) {
				glTexParameterf(bindTarget, GL_TEXTURE_WRAP_T, gl_clamp_val); CHECKGL;
			}
		}
		if (rtex->mirror_s) {
			glTexParameterf(bindTarget, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_IBM); CHECKGL;
		}
		if (rtex->mirror_t) {
			glTexParameterf(bindTarget, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT_IBM); CHECKGL;
		}
		if (rtex->point_sample) {
			glTexParameterf(bindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST); CHECKGL;
			glTexParameterf(bindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST); CHECKGL;
		} else {
			glTexParameterf(bindTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECKGL;
			if (rtex->mipmap) {
				glTexParameterf(bindTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); CHECKGL;
			} else {
				glTexParameterf(bindTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECKGL;
			}
			if (rdr_caps.supports_anisotropy) {
				assert(rdr_view_state.texAnisotropic >= 1.0f);
				glTexParameterf(bindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, rdr_view_state.texAnisotropic); CHECKGL;
			}
		}
	}
	// download the bitmap data
	if (rtex->from_screen)
	{
		/*
		if (rtex->sub_image)
			glCopyTexSubImage2D(target, 0, 0, 0, rtex->x, rtex->y, rtex->width, rtex->height); CHECKGL;
		else
			glCopyTexImage2D(target, 0, rtex->dst_format, rtex->x, rtex->y, rtex->width, rtex->height, rtex->tex_border); CHECKGL;
		*/

		glTexImage2D(target, 0, GL_RGBA8, rtex->width, rtex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); CHECKGL;
		winCopyFrameBufferDirect(target, rtex->id, 0, rtex->x, rtex->y, rtex->width, rtex->height, 0, false, false);

	} else if (!rtex->dxtc)
	{
		// Raw data
		if (rtex->mipmap) {
			COH_Build2DMipmapsSRC( target,rtex->dst_format,rtex->width,rtex->height,rtex->src_format,GL_UNSIGNED_BYTE,bitmap); CHECKGL;
		} else if (rtex->floatingpoint) {
			glTexImage2D( target, 0, rtex->dst_format, rtex->width, rtex->height, rtex->tex_border, rtex->src_format, GL_FLOAT, bitmap ); CHECKGL;
		} else {
			glTexImage2D( target, 0, rtex->dst_format, rtex->width, rtex->height, rtex->tex_border, rtex->src_format, GL_UNSIGNED_BYTE, bitmap ); CHECKGL;
		}
	}
	else
	{
		// DDS source data (BGRA)
		int	offset,blockSize,size;

		offset   = 0;
		blockSize = (rtex->src_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

		/* load the mipmaps */
		for (i = 0; i <= rtex->mip_count && (width || height); ++i)
		{
			if (width == 0)
				width = 1;
			if (height == 0)
				height = 1;

			switch (rtex->src_format)
			{
				case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT :
				case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT :
				case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT :
					//compressed rgba
					size = ((width+3)/4)*((height+3)/4)*blockSize;
					glCompressedTexImage2DARB(target, i, rtex->src_format, width, height, rtex->tex_border, size, bitmap + offset); CHECKGL;
					break;

				case TEXFMT_ARGB_8888 :
					// uncompressed rgba
					size = width*height*4;
					
					if (GLEW_VERSION_1_2 || GLEW_EXT_bgra)
					{
						// GL_BGRA is "available only if the GL version is 1.2 or greater."
						glTexImage2D( target, i, GL_RGBA8, width, height, rtex->tex_border, GL_BGRA, GL_UNSIGNED_BYTE, bitmap + offset ); CHECKGL;
					}
					else
					{
						int		j;
						U8		c,*pix = bitmap + offset;

						// Convert from BGRA to RGBA
						for(j=0;j<size;j+=4)
						{
							c = pix[j];
							pix[j] = pix[j+2];
							pix[j+2] = c;
						}
						glTexImage2D( target, i, GL_RGBA8, width, height, rtex->tex_border, GL_RGBA, GL_UNSIGNED_BYTE, bitmap + offset ); CHECKGL;
					}
					break;

				case TEXFMT_ARGB_0888 :
					// uncompressed rgb
					size = width*height*3;

					if (GLEW_VERSION_1_2 || GLEW_EXT_bgra)
					{
						// GL_BGR is "available only if the GL version is 1.2 or greater."
						glTexImage2D( target, i, GL_RGB8, width, height, rtex->tex_border, GL_BGR, GL_UNSIGNED_BYTE, bitmap + offset ); CHECKGL;
					}
					else
					{
						int		j;
						U8		c,*pix = bitmap + offset;

						// Convert from BGR to RGB
						for(j=0;j<size;j+=3)
						{
							c = pix[j];
							pix[j] = pix[j+2];
							pix[j+2] = c;
						}
						glTexImage2D( target, i, GL_RGB8, width, height, rtex->tex_border, GL_RGB, GL_UNSIGNED_BYTE, bitmap + offset ); CHECKGL;
					}
					break;

				/* UNTESTED */
				case TEXFMT_ARGB_1555 :
					size = width*height*2;

					// GL_UNSIGNED_SHORT_5_5_5_1 is "available only if the GL version is 1.2 or greater."
					if ((GLEW_VERSION_1_2 || GLEW_EXT_packed_pixels) && GLEW_EXT_bgra)
					{
						glTexImage2D( target, i, GL_RGB5_A1, width, height, rtex->tex_border, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, bitmap + offset ); CHECKGL;
					}
					else
					{
						// convert to something OpenGL 1.1 supports
						int		j;
						U16		c;
						U16		*old_pix = (U16*) (bitmap + offset);
						U8		*new_bitmap = malloc(width*height*4);
						U8		*new_pix = new_bitmap;

						// Convert from A1R5G5B5 to RGBA8
						for(j=0; j<size/2; j++,old_pix++,new_pix+=4)
						{
							c = *old_pix;

							new_pix[0] = (c & 0x7C00) >> 7;
							new_pix[1] = (c & 0x03E0) >> 2;
							new_pix[2] = (c & 0x001F) << 3;

							// Replicate the top bits of each component into the bottom 4 bits of each component
							new_pix[0] |= new_pix[0] >> 5;
							new_pix[1] |= new_pix[1] >> 5;
							new_pix[2] |= new_pix[2] >> 5;

							new_pix[3] = (c & 0x8000) ? 0xFF : 0;
						}

						glTexImage2D( target, i, GL_RGB5_A1, width, height, rtex->tex_border, GL_RGBA, GL_UNSIGNED_BYTE, new_bitmap ); CHECKGL;

						free(new_bitmap);
					}

					break;

				case TEXFMT_ARGB_0565 :
					size = width*height*2;

					// GL_UNSIGNED_SHORT_5_6_5 is "available only if the GL version is 1.2 or greater."
					if ((GLEW_VERSION_1_2 || GLEW_EXT_packed_pixels) && GLEW_EXT_bgra)
					{
#if 0
						// OpenGL does not have an GL_R5_G6_B5 internal format
						//glTexImage2D( target, i, GL_R5_G6_B5, width, height, rtex->tex_border, GL_BGRA, GL_UNSIGNED_SHORT_5_6_5_REV, bitmap + offset ); CHECKGL;
#else
						// convert to something OpenGL 1.1 supports
						int		j;
						U16		c;
  						U16		*pix = (U16*) (bitmap + offset);

						// Convert from R5G6B5 to A1R5G5B5
						for(j=0; j<size/2; j++,pix++)
						{
							c = *pix;

							*pix = 0x8000 | ((c & 0xF800) >> 1) | ((c & 0x07D0) >> 1) | (c & 0x001F);
						}

						glTexImage2D( target, i, GL_RGB5_A1, width, height, rtex->tex_border, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, bitmap + offset ); CHECKGL;
#endif
					}
					else
					{
						// convert to something OpenGL 1.1 supports
						int		j;
						U16		c;
						U16		*old_pix = (U16*) (bitmap + offset);
						U8		*new_bitmap = malloc(width*height*3);
						U8		*new_pix = new_bitmap;

						// Convert from R5G6B5 to RGB8
						for(j=0; j<size/2; j++,old_pix++,new_pix+=3)
						{
							c = *old_pix;

							new_pix[0] = (c & 0xF800) >> 8;
							new_pix[1] = (c & 0x07E0) >> 3;
							new_pix[2] = (c & 0x001F) << 3;

							// Replicate the top bits of each component into the bottom bits of each component
							new_pix[0] |= new_pix[0] >> 5;
							new_pix[1] |= new_pix[1] >> 6;
							new_pix[2] |= new_pix[2] >> 5;
						}

						glTexImage2D( target, i, GL_RGB5, width, height, rtex->tex_border, GL_RGB, GL_UNSIGNED_BYTE, new_bitmap ); CHECKGL;

						free(new_bitmap);
					}
					break;

				case TEXFMT_ARGB_4444 :
					size = width*height*2;

					// GL_UNSIGNED_SHORT_4_4_4_4 is "available only if the GL version is 1.2 or greater."
					if ((GLEW_VERSION_1_2 || GLEW_EXT_packed_pixels) && GLEW_EXT_bgra)
					{
						glTexImage2D( target, i, GL_RGBA4, width, height, rtex->tex_border, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV, bitmap + offset ); CHECKGL;
					}
					else
					{
						// convert to something OpenGL 1.1 supports
						int		j;
						U16		c;
						U16		*old_pix = (U16*) (bitmap + offset);
						U8		*new_bitmap = malloc(width*height*4);
						U8		*new_pix = new_bitmap;

						// Convert from ARGB4 to RGBA8
						for(j=0; j<size/2; j++,old_pix++,new_pix+=4)
						{
							U32 *new_pix32 = (U32*)new_pix;

							c = *old_pix;

							new_pix[0] = (c & 0x0F00) >> 4;
							new_pix[1] = (c & 0x00F0);
							new_pix[2] = (c & 0x000F) << 4;
							new_pix[3] = (c & 0xF000) >> 8;

							// Replicate the top 4 bits of each component into the bottom 4 bits of each component
							*new_pix32 |= *new_pix32 >> 4;
						}

						glTexImage2D( target, i, GL_RGBA4, width, height, rtex->tex_border, GL_RGBA, GL_UNSIGNED_BYTE, new_bitmap ); CHECKGL;

						free(new_bitmap);
					}
					break;

				/* UNTESTED */
				/*
				case TEXFMT_I_8 :
					size = width*height*2;
					glTexImage2D( target, i, GL_LUMINANCE8, width, height, rtex->tex_border, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap + offset ); CHECKGL;
					break;
				*/

				default: 
					assert("Unhandled rtex->src_format in texCopyDirect()" == 0);
			}

			offset += size;
			width  >>= 1;
			height >>= 1;
		}
	}
	rdrEndMarker();
}

void texSubCopyDirect(RdrSubTexParams *rtex)
{
	int		target = rtex->target;
	U32		width = rtex->width, height=rtex->height;
	U8		*bitmap = (U8*)(rtex+1);

	rdrBeginMarker(__FUNCTION__);

	if (!target)
		target = GL_TEXTURE_2D;

	WCW_BindTexture(GL_TEXTURE_2D, 0, rtex->id);
	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECKGL;

	// download the bitmap data
	if (!rtex->dxtc)
	{
		// RGB, RGBA, or LUMINANCE_ALPHA
		glTexSubImage2D( target, 0, rtex->x_offset, rtex->y_offset, rtex->width, rtex->height, rtex->format, GL_UNSIGNED_BYTE, bitmap ); CHECKGL;
	}
	else
	{
		// DDS source data (BGRA)
		int	blockSize,size;

		blockSize = (rtex->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;

		if (rtex->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
			rtex->format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
			rtex->format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		{
			//compressed rgba
			size = ((width+3)/4)*((height+3)/4)*blockSize;
			glCompressedTexSubImage2DARB(target, 0, rtex->x_offset, rtex->y_offset, width, height, rtex->format, size, bitmap); CHECKGL;
		}
		else
		{
			// uncompressed bgra
			size = width*height*4;
			{
				int		i;
				U8		c,*pix = bitmap;

				for(i=0;i<size;i+=4)
				{
					c = pix[i];
					pix[i] = pix[i+2];
					pix[i+2] = c;
				}
			}

			glTexSubImage2D( target, 0, rtex->x_offset, rtex->y_offset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap ); CHECKGL;
		}
	}
	rdrEndMarker();
}

void texBindTexCoordPointerEx(TexLayerIndex texindex, int stride, const void *pointer)
{
	if (rdr_caps.chip != TEX_ENV_COMBINE) {
		WCW_TexCoordPointer(texindex, 2, GL_FLOAT, stride, pointer);
	}
	texCoordPointers[texindex] = pointer;
	texCoordStrides[texindex] = stride;
	g_renderstate_dirty = true;
}

void texEnableTexCoordPointer(TexLayerIndex texindex, bool enable)
{
	if (rdr_caps.chip != TEX_ENV_COMBINE) {
		if (enable) {
			WCW_EnableClientState(GLC_TEXTURE_COORD_ARRAY_0 + texindex);
		} else {
			WCW_DisableClientState(GLC_TEXTURE_COORD_ARRAY_0 + texindex);
		}
	}
	texCoordPointersEnabled[texindex] = enable;
	g_renderstate_dirty = true;
}

/* Trivial polymorph I want to get rid of to tidy up state management code */
void texSetWhite(TexLayerIndex texindex)
{
	if (!rdr_view_state.white_tex_id)
		return;

	texBind(texindex, rdr_view_state.white_tex_id);
}

void texBind(TexLayerIndex texindex, int texid)
{
	texBindCalls++;

	if (!texid)
		return;

	if (rdr_caps.chip != TEX_ENV_COMBINE && texindex < rdr_caps.num_texunits) {
		// We apply these at the end for TEX_ENV_COMBINE mode
		WCW_BindTextureFast(GL_TEXTURE_2D, texindex, texid);
	}

	boundTextures[texindex] = texid;
	g_renderstate_dirty = true;
}

// For cubemaps, should probably combine with texBind above and pass target type
void texBindCube(TexLayerIndex texindex, int texid)
{
	texBindCalls++;

	if (!texid)
		return;

	if (rdr_caps.chip != TEX_ENV_COMBINE && texindex < rdr_caps.num_texunits) {
		// We apply these at the end for TEX_ENV_COMBINE mode
		WCW_BindTextureFast(GL_TEXTURE_CUBE_MAP_ARB, texindex, texid);
	}

	boundTextures[texindex] = texid;
	g_renderstate_dirty = true;
}

void rdrGetTexInfoDirect(RdrGetTexInfo *get)
{
	WCW_BindTexture(GL_TEXTURE_2D, 0, get->texid);

	if (get->data)
	{
		glGetTexImage(GL_TEXTURE_2D, 0, get->pixelformat, get->pixeltype, get->data); CHECKGL;
	}
	else
	{
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, get->widthout); CHECKGL;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, get->heightout); CHECKGL;
	}
}

void texSetAnisotropicDirect(int value)
{
	rdr_view_state.texAnisotropic = value;
}

void texResetAnisotropicDirect(int texid, bool bCubemap)
{
	if (rdr_caps.supports_anisotropy)
	{
		GLenum target = bCubemap ? GL_TEXTURE_CUBE_MAP: GL_TEXTURE_2D;
		WCW_BindTexture(target, 0, texid);
		glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)rdr_view_state.texAnisotropic); CHECKGL;
	}
}


void rdrTexFreeDirect(int texid)
{
	PERFINFO_AUTO_START("WCW_ResetTextureState", 1);
	WCW_ResetTextureState();
	PERFINFO_AUTO_STOP_START("glDeleteTextures", 1);
	glDeleteTextures(1,&texid); CHECKGL;
	PERFINFO_AUTO_STOP();
}

