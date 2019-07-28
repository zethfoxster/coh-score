#include <wininclude.h>
#include <winuser.h>
#include "pbuffer.h"
#include "tex.h"
#include "rt_state.h"
#include "renderUtil.h"
#include "win_init.h"
#include "cmdgame.h"
#include "timing.h"
#include "win_cursor.h"
#include "textureatlas.h"
#include "Color.h"
#include "osdependent.h"
#include "utils.h"
#include "dd.h"
#include "file.h" // for isDevelopmentMode()

#define CURSOR_SWBLENDING
//#define CURSOR_SWBLENDING_DUMP "C:\\temp\\swcursor\\"

#ifdef CURSOR_SWBLENDING_DUMP
#include <file.h>
#endif

#define RGBA_R(rgba) ((unsigned char) (((rgba) >> 24) & 0xff))
#define RGBA_G(rgba) ((unsigned char) (((rgba) >> 16) & 0xff))
#define RGBA_B(rgba) ((unsigned char) (((rgba) >> 8)  & 0xff))
#define RGBA_A(rgba) ((unsigned char) (((rgba) >> 0)  & 0xff))

static HCURSOR	curse;
static int		curseNoDestroy;
int g_cursor_size = 32;
static int curr_hotspot_x,curr_hotspot_y;

#ifdef CURSOR_SWBLENDING
#define MAX_CURSOR_SIZE 64
static U8 cursor_pixmap[MAX_CURSOR_SIZE*MAX_CURSOR_SIZE*4];
static bool cursor_pixmap_is_clear = true;
#endif

typedef struct
{
	BITMAPV5HEADER	bi;
	U32				palette[256];
} BMAP;


void hwcursorClear()
{
#ifdef CURSOR_SWBLENDING
	memset(cursor_pixmap, 0, g_cursor_size*g_cursor_size*4);
	cursor_pixmap_is_clear = true;
#else
	rdrQueueCmd(DRAWCMD_CURSORCLEAR);
#endif
}

int supportsGdiPlus() 
{
	DWORD	version;

	version = GetVersion();
	if (version & 0x80000000)
		return 0;
	if ((version & 255) < 5)
		return 0;
	return 1;
} 

#ifdef CURSOR_SWBLENDING_DUMP
// Debug code to dump a 32-bpp pixmap to a bmp file
static void dumpPixmapBmp(const char *filename, int width, int height, int bpp, const U8 *pixbuf)
{
	U8 *pixrow;
	FILE	*file;
	int int_4;
	short int_2;
	char int_1;
	int bytespp = (bpp + 7) >> 3;

	assert(bpp == 24 || bpp == 32);

	file = fopen(filename,"wb");
	if (file)
	{
		int width4 = ((width * bytespp) + 3) & ~3;
		int pad = width4 - (width * bytespp);
		int row, column;
		const int header2size = 40;

		// bmp header
		int_1 = 'B';
		fwrite(&int_1,1,1,file);
		int_1 = 'M';
		fwrite(&int_1,1,1,file);

		int_4 = width4 * height + 14 + header2size;	// filesize
		fwrite(&int_4,4,1,file);

		int_2 = 0;					// reserved
		fwrite(&int_2,2,1,file);

		int_2 = 0;					// reserved
		fwrite(&int_2,2,1,file);

		int_4 = 14 + header2size;			// offset to bitmap data
		fwrite(&int_4,4,1,file);

		// DIB header (Windows V3)
		int_4 = 40;					// header size
		fwrite(&int_4,4,1,file);

		int_4 = width;				// width
		fwrite(&int_4,4,1,file);

		int_4 = height;				// height
		fwrite(&int_4,4,1,file);

		int_2 = 1;					// num color planes
		fwrite(&int_2,2,1,file);

		int_2 = bpp;				// bpp
		fwrite(&int_2,2,1,file);

		int_4 = 0;					// compression
		fwrite(&int_4,4,1,file);

		int_4 = width4 * height;	// raw image size
		fwrite(&int_4,4,1,file);

		int_4 = 2835;				// horizontal pixels per meter
		fwrite(&int_4,4,1,file);

		int_4 = 2835;				// vertical pixels per meter
		fwrite(&int_4,4,1,file);

		int_4 = 0;					// num colors in palette
		fwrite(&int_4,4,1,file);

		int_4 = 0;					// num important colors used
		fwrite(&int_4,4,1,file);

		// raw image data
		// each row needs to be padded out to 4 bytes
		int_4 = 0;					// pad
		pixrow = malloc(width * bytespp);

		for (row = 0; row < height; row++)
		{
			const U8 *pCurrentSrc = pixbuf + row * width * bytespp;
			U8 *pCurrentDest = pixrow;

			// Need to reverse from RGB to BGR
			for (column = 0; column < width; column++)
			{
				pCurrentDest[0] = pCurrentSrc[2];
				pCurrentDest[1] = pCurrentSrc[1];
				pCurrentDest[2] = pCurrentSrc[0];

				if (bpp == 32)
					pCurrentDest[3] = pCurrentSrc[3];

				pCurrentSrc += bytespp;
				pCurrentDest += bytespp;
			}

			fwrite(pixrow, width * bytespp, 1, file);

			if (pad)
				fwrite(&int_4,pad,1,file);
		}

		free(pixrow);

		fclose(file);
	}
}
#endif

void hwcursorBlit( AtlasTex *bind, int width, int height, F32 x, F32 y, F32 scale, int clr )
{
	static const F32 inv255 = 1.0f / 255.0f;

	if (!supportsGdiPlus() && !game_state.compatibleCursors)
	{
		x += (g_cursor_size/2) - curr_hotspot_x;
		y += (g_cursor_size/2) - curr_hotspot_y;
	}

	width = MIN(width, bind->width);
	height = MIN(height, bind->height);

	width *= scale;
	height *= scale;

	assert(width > 0 && height > 0);

#ifdef CURSOR_SWBLENDING
	{
		BasicTexture *texture = 0;
		TexReadInfo *rawInfo = 0;
		U8 *rawPixmap;
		DDSURFACEDESC2 *ddsheader;
		int x1 = x;
		int x2 = x + width;
		int y1 = y;
		int y2 = y + height;
#ifdef CURSOR_SWBLENDING_DUMP
		static dmpCnt = 0;
#endif
		bool rawTextureOK = true;
		char errorString[1000];

		x1 = MAX(x1, 0);
		y1 = MAX(y1, 0);

		texture = texLoadRawData(bind->name, TEX_LOAD_NOW_CALLED_FROM_MAIN_THREAD, TEX_FOR_UI);
		assert(texture);

		rawInfo = texture->actualTexture->rawInfo;
		assert(rawInfo);

		// Skip DDS header
		ddsheader = (DDSURFACEDESC2*) (rawInfo->data + 4);

		if (rawInfo->mip_count > 0 && isDevelopmentOrQAMode())
		{
			sprintf(errorString, "Cursor texture or icon '%s' should not have mipmaps", bind->name);

			//MessageBox(0, buf, "Warning", MB_ICONERROR );
			//printf("%s\n", buf);
			winMsgAlert(errorString);

		}

		if (rawInfo->size <= 8 || (((int)ddsheader->dwSize + rawInfo->width * rawInfo->height * 4) > rawInfo->size))
		{
			sprintf(errorString, "Bad rawInfo size of %d bytes for cursor texture or icon '%s' of %dx%d size\n", rawInfo->size, bind->name, rawInfo->width, rawInfo->height);
			winMsgAlert(errorString);
			rawTextureOK = false;
		}

		if (ddsheader->ddpfPixelFormat.dwSize != 32)
		{
			sprintf(errorString, "Cursor texture or icon '%s' was %d bpp instead of 32 bpp\n", bind->name, ddsheader->ddpfPixelFormat);
			winMsgAlert(errorString);
			rawTextureOK = false;
		}

		if (ddsheader->dwWidth != rawInfo->width || ddsheader->dwHeight != rawInfo->height)
		{
			sprintf(errorString, "Inconsistency between dds header width/height of %dx%d and rawInfo width/height of %dx%d for cursor texture or icon '%s'",
					ddsheader->dwWidth, ddsheader->dwHeight, rawInfo->width, rawInfo->height, bind->name);
			winMsgAlert(errorString);
			rawTextureOK = false;
		}

		if (rawTextureOK)
		{
			rawPixmap = rawInfo->data + 4 + ((U32*)rawInfo->data)[1];

			assert(rawInfo->width >= bind->width);
			assert(rawInfo->height >= bind->height);

#ifdef CURSOR_SWBLENDING_DUMP
			{
				char filename[255];
				sprintf(filename, CURSOR_SWBLENDING_DUMP "cursordump_%d_0.bmp", dmpCnt);
				dumpPixmapBmp(filename, g_cursor_size, g_cursor_size, 32, cursor_pixmap);

				sprintf(filename, CURSOR_SWBLENDING_DUMP "cursordump_%d_1_scale_%4.2f_offset_%d_%d.bmp", dmpCnt, scale, (int)x, (int)y);
				dumpPixmapBmp(filename, rawInfo->width, rawInfo->height, 32, rawPixmap);
			}
#endif

			if (cursor_pixmap_is_clear &&  width == bind->width && height == bind->height && clr == 0xFFFFFFFF
				   && x2 < g_cursor_size && y2 < g_cursor_size)
			{
				int i;
				int dstYOffset = (g_cursor_size - 1) * g_cursor_size * 4;
				int srcYOffset = 0;

				// simple case
				for (i = 0; i < height; i++)
				{
#if 0
					// We can't do this simple memcpy because we want the color flipping
					memcpy(&cursor_pixmap[(g_cursor_size - 1 - i) * g_cursor_size * 4], &rawPixmap[i * 4 * rawInfo->width], 4 * rawInfo->width);
#else
					int j;
					int dstOffset = dstYOffset + x1 * 4;
					int srcOffset = srcYOffset;

					for (j = 0; j < width; j++)
					{
						// Flip source color order here
						cursor_pixmap[dstOffset + 0] = rawPixmap[srcOffset + 2];
						cursor_pixmap[dstOffset + 1] = rawPixmap[srcOffset + 1];
						cursor_pixmap[dstOffset + 2] = rawPixmap[srcOffset + 0];
						cursor_pixmap[dstOffset + 3] = rawPixmap[srcOffset + 3];

						dstOffset += 4;
						srcOffset += 4;
					}

					dstYOffset -= g_cursor_size * 4;
					srcYOffset += rawInfo->width * 4;
#endif
				}
			}
			else
			{
				int raw_width_minus_one = rawInfo->width - 1;
				int raw_height_minus_one = rawInfo->height - 1;
				int cursor_size_minus_one = g_cursor_size - 1;
				int i;

				F32 srcXstep = ((F32)bind->height - 1) / ((F32)height - 1); 
				F32 srcYstep = ((F32)bind->width - 1) / ((F32)width - 1); 
				F32 srcY = 0.0f;

				F32 colorScale[4];

				colorScale[0] = RGBA_R(clr) * inv255;
				colorScale[1] = RGBA_G(clr) * inv255;
				colorScale[2] = RGBA_B(clr) * inv255;
				colorScale[3] = RGBA_A(clr) * inv255;

				for (i = y1; i < y2; i++, srcY += srcYstep)
				{
					int intY = (int)srcY;
					int intY0 = CLAMP(intY, 0, raw_height_minus_one);
					int intY1 = CLAMP(intY + 1, 0, raw_height_minus_one);
					int Y0offset = intY0 * rawInfo->width;
					int Y1offset = intY1 * rawInfo->width;
					F32 lerpy0 = 1.0f - CLAMP(srcY - intY0, 0, 1);
					F32 lerpy1 = 1.0f - CLAMP(intY1 - srcY, 0, 1);
					int j;
					F32 srcX = 0.0f;

					if (i < 0 || i > cursor_size_minus_one)
						continue;

					for (j = x1; j < x2; j++, srcX += srcXstep)
					{
						F32 fsample[4];
						int intX = (int)srcX;
						int intX0 = CLAMP(intX, 0, raw_width_minus_one);
						int intX1 = CLAMP(intX + 1, 0, raw_width_minus_one);
						F32 lerpx0 = 1.0f - CLAMP(srcX - intX0, 0, 1);
						F32 lerpx1 = 1.0f - CLAMP(intX1 - srcX, 0, 1);
						int sampleIndex;
						F32 lerpAmount;
						F32 blendLerp;
						F32 invBlendLerp;
						F32 fDstSample[4];
						F32 srcAlpha;
						F32 dstAlpha;
						F32 finalAlpha;

						if (j < 0 || j > cursor_size_minus_one)
							continue;

						// Get 4 nearest samples and filter
						sampleIndex = (Y0offset + intX0) * 4;
						lerpAmount = lerpy0 * lerpx0;
						fsample[0]  = ((F32) rawPixmap[sampleIndex + 0]) * lerpAmount;
						fsample[1]  = ((F32) rawPixmap[sampleIndex + 1]) * lerpAmount;
						fsample[2]  = ((F32) rawPixmap[sampleIndex + 2]) * lerpAmount;
						fsample[3]  = ((F32) rawPixmap[sampleIndex + 3]) * lerpAmount;
	 
						sampleIndex = (Y0offset + intX1) * 4;
						lerpAmount = lerpy0 * lerpx1;
						fsample[0] += ((F32) rawPixmap[sampleIndex + 0]) * lerpAmount;
						fsample[1] += ((F32) rawPixmap[sampleIndex + 1]) * lerpAmount;
						fsample[2] += ((F32) rawPixmap[sampleIndex + 2]) * lerpAmount;
						fsample[3] += ((F32) rawPixmap[sampleIndex + 3]) * lerpAmount;

						sampleIndex = (Y1offset + intX0) * 4;
						lerpAmount = lerpy1 * lerpx0;
						fsample[0] += ((F32) rawPixmap[sampleIndex + 0]) * lerpAmount;
						fsample[1] += ((F32) rawPixmap[sampleIndex + 1]) * lerpAmount;
						fsample[2] += ((F32) rawPixmap[sampleIndex + 2]) * lerpAmount;
						fsample[3] += ((F32) rawPixmap[sampleIndex + 3]) * lerpAmount;

						sampleIndex = (Y1offset + intX1) * 4;
						lerpAmount = lerpy1 * lerpx1;
						fsample[0] += ((F32) rawPixmap[sampleIndex + 0]) * lerpAmount;
						fsample[1] += ((F32) rawPixmap[sampleIndex + 1]) * lerpAmount;
						fsample[2] += ((F32) rawPixmap[sampleIndex + 2]) * lerpAmount;
						fsample[3] += ((F32) rawPixmap[sampleIndex + 3]) * lerpAmount;

						// Scale by color
						fsample[0] *= colorScale[0];
						fsample[1] *= colorScale[1];
						fsample[2] *= colorScale[2];
						fsample[3] *= colorScale[3];

						// Get destination color
						sampleIndex = ((g_cursor_size - 1 - i) * g_cursor_size + j) * 4;
						fDstSample[0] = cursor_pixmap[sampleIndex + 0];
						fDstSample[1] = cursor_pixmap[sampleIndex + 1];
						fDstSample[2] = cursor_pixmap[sampleIndex + 2];
						fDstSample[3] = cursor_pixmap[sampleIndex + 3];

						// Calculate final alpha
						srcAlpha = fsample[3] * inv255;
						dstAlpha = fDstSample[3] * inv255;
						finalAlpha = srcAlpha + dstAlpha - (srcAlpha * dstAlpha);

						// Blend
						if (finalAlpha == 0.0f)
							blendLerp = 0.0f;
						else
							blendLerp = srcAlpha / finalAlpha;
						invBlendLerp = 1.0f - blendLerp;

						// Flip source color order here
						fDstSample[0] = fsample[2] * blendLerp + fDstSample[0] * invBlendLerp;
						fDstSample[1] = fsample[1] * blendLerp + fDstSample[1] * invBlendLerp;
						fDstSample[2] = fsample[0] * blendLerp + fDstSample[2] * invBlendLerp;

						// Write destination color
						cursor_pixmap[sampleIndex + 0] = fDstSample[0] + 0.5f;
						cursor_pixmap[sampleIndex + 1] = fDstSample[1] + 0.5f;
						cursor_pixmap[sampleIndex + 2] = fDstSample[2] + 0.5f;
						cursor_pixmap[sampleIndex + 3] = finalAlpha * 255.0f + 0.5f;
					}
				}
			}

#ifdef CURSOR_SWBLENDING_DUMP
			{
				char filename[255];
				sprintf(filename, CURSOR_SWBLENDING_DUMP "cursordump_%d_2.bmp", dmpCnt);
				dumpPixmapBmp(filename, g_cursor_size, g_cursor_size, 32, cursor_pixmap);
				dmpCnt++;
			}
#endif
		}
		else
		{
			// raw DDS data does not look good.  May be the white texture.  Assume it is an all white texture
			int i;
			int cursor_size_minus_one = g_cursor_size - 1;
			F32 colorScale[4];

			colorScale[0] = RGBA_R(clr) * inv255;
			colorScale[1] = RGBA_G(clr) * inv255;
			colorScale[2] = RGBA_B(clr) * inv255;
			colorScale[3] = RGBA_A(clr) * inv255;

			for (i = y1; i < y2; i++)
			{
				int j;

				if (i < 0 || i > cursor_size_minus_one)
					continue;

				for (j = x1; j < x2; j++)
				{
					int sampleIndex;
					F32 srcAlpha;
					F32 dstAlpha;
					F32 finalAlpha;

					if (j < 0 || j > cursor_size_minus_one)
						continue;

					sampleIndex = ((g_cursor_size - 1 - i) * g_cursor_size + j) * 4;

					srcAlpha = colorScale[3];
					dstAlpha = cursor_pixmap[sampleIndex + 3] * inv255;
					finalAlpha = srcAlpha + dstAlpha - (srcAlpha * dstAlpha);

					// Write destination color
					cursor_pixmap[sampleIndex + 0] = cursor_pixmap[sampleIndex + 0] * colorScale[0] + 0.5f;
					cursor_pixmap[sampleIndex + 1] = cursor_pixmap[sampleIndex + 1] * colorScale[1] + 0.5f;
					cursor_pixmap[sampleIndex + 2] = cursor_pixmap[sampleIndex + 2] * colorScale[2] + 0.5f;
					cursor_pixmap[sampleIndex + 3] = finalAlpha * 255.0f + 0.5f;
				}
			}
		}

		texUnloadRawData(texture);
		cursor_pixmap_is_clear = false;
	}
#else // CURSOR_SWBLENDING
	{
		F32		x1, x2, y1, y2, v_size;
		U8		color[4];
		int size = sizeof(RdrSpritesCmd) + 4 * sizeof(RdrSpriteVertex);
		RdrSpritesCmd *cmd;
		RdrSpriteVertex *vertices;
		int texid;
		F32		u1, u2, v1, v2;

		v_size = cursor_pbuf.virtual_height;

		x1 = x;
		x2 = x + width;
		y1 = v_size - 1 - y;
		y2 = v_size - 1 - (y + height);

		color[0] = RGBA_R(clr);
		color[1] = RGBA_G(clr);
		color[2] = RGBA_B(clr);
		color[3] = RGBA_A(clr);

		texid = atlasDemandLoadTexture(bind);

		atlasGetModifiedUVs(bind, 0, 0, &u1, &v1);
		atlasGetModifiedUVs(bind, 1, 1, &u2, &v2);

		cmd = rdrQueueAlloc(DRAWCMD_CURSORBLIT, size);
		vertices = (RdrSpriteVertex *)(cmd+1);
		cmd->sprite_count = 1;
		cmd->state.texid = texid;
		cmd->state.target = 0x0DE1; //GL_TEXTURE_2D;
		cmd->state.additive = 0;
		cmd->state.use_scissor = 0;

		// increase x first
		vertices[0].point[0] = x1;
		vertices[0].point[1] = y1;
		vertices[0].point[2] = 0;
		memcpy(vertices[0].rgba, color, sizeof(color));
		vertices[0].texcoord[0] = u1;
		vertices[0].texcoord[1] = v1;

		vertices[1].point[0] = x2;
		vertices[1].point[1] = y1;
		vertices[1].point[2] = 0;
		memcpy(vertices[1].rgba, color, sizeof(color));
		vertices[1].texcoord[0] = u2;
		vertices[1].texcoord[1] = v1;

		vertices[2].point[0] = x2;
		vertices[2].point[1] = y2;
		vertices[2].point[2] = 0;
		memcpy(vertices[2].rgba, color, sizeof(color));
		vertices[2].texcoord[0] = u2;
		vertices[2].texcoord[1] = v2;

		vertices[3].point[0] = x1;
		vertices[3].point[1] = y2;
		vertices[3].point[2] = 0;
		memcpy(vertices[3].rgba, color, sizeof(color));
		vertices[3].texcoord[0] = u1;
		vertices[3].texcoord[1] = v2;

		rdrQueueSend();
	}
#endif // CURSOR_SWBLENDING
}

//Please call this function before you blit to the hardware cursor, it keeps the framerate from tanking
//To force an update, call it twice, first with zeros.  
//If you add another blit overlay, you need to add another parameter. Thank you, drive though.
int iconIsNew( AtlasTex * base_spr, AtlasTex * cursor_dragged_icon )
{
	static AtlasTex * curr_base_spr				= 0;
	static AtlasTex * curr_cursor_dragged_icon	= 0;
	static int last_compatibleCursors = 0;
	int newIcon = 0;

	if( curr_base_spr != base_spr || curr_cursor_dragged_icon != cursor_dragged_icon || game_state.compatibleCursors != last_compatibleCursors)
	{
		newIcon = 1;
		curr_base_spr				= base_spr;
		curr_cursor_dragged_icon	= cursor_dragged_icon;
		last_compatibleCursors		= game_state.compatibleCursors;
	}

	return newIcon;
}

// Create a custom cursor at run time. 


void hwcursorSetHotSpot(int hot_x,int hot_y)
{
	curr_hotspot_x = hot_x;
	curr_hotspot_y = hot_y;
}


static int findNearestIdx(const U8 *c,PALETTEENTRY *syspals,int num_sys)
{
	int		i,dist,mindist=0x7fffffff,idx=0;

	for(i=0;i<num_sys;i++)
	{
		dist = ABS(c[0] - syspals[i].peRed);
		dist += ABS(c[1] - syspals[i].peGreen);
		dist += ABS(c[2] - syspals[i].peBlue);
		if (dist < mindist && (i < 10 || i > 245))// && (syspals[i].peRed == syspals[i].peGreen && syspals[i].peBlue == syspals[i].peGreen))
		{
			mindist = dist;
			idx = i;
		}
	}
	return idx;// ? 248 : 0;
}

static HCURSOR CreateCompatibleCursor(const U8 *pixdata,int hotspot_x,int hotspot_y)
{
	DWORD dwWidth = g_cursor_size; // width of cursor
	DWORD dwHeight = g_cursor_size; // height of cursor
	HCURSOR hCurs1;
	int size = (g_cursor_size*g_cursor_size+7)/8;
	U8 *andmask = _alloca(size);
	U8 *xormask = _alloca(size);
	int i, j, bitc=0;

	BYTE ANDmaskCursor[] = 
	{ 
		0xFF, 0xFC, 0x3F, 0xFF,   // line 1 
			0xFF, 0xC0, 0x1F, 0xFF,   // line 2 
			0xFF, 0x00, 0x3F, 0xFF,   // line 3 
			0xFE, 0x00, 0xFF, 0xFF,   // line 4 

			0xF7, 0x01, 0xFF, 0xFF,   // line 5 
			0xF0, 0x03, 0xFF, 0xFF,   // line 6 
			0xF0, 0x03, 0xFF, 0xFF,   // line 7 
			0xE0, 0x07, 0xFF, 0xFF,   // line 8 

			0xC0, 0x07, 0xFF, 0xFF,   // line 9 
			0xC0, 0x0F, 0xFF, 0xFF,   // line 10 
			0x80, 0x0F, 0xFF, 0xFF,   // line 11 
			0x80, 0x0F, 0xFF, 0xFF,   // line 12 

			0x80, 0x07, 0xFF, 0xFF,   // line 13 
			0x00, 0x07, 0xFF, 0xFF,   // line 14 
			0x00, 0x03, 0xFF, 0xFF,   // line 15 
			0x00, 0x00, 0xFF, 0xFF,   // line 16 

			0x00, 0x00, 0x7F, 0xFF,   // line 17 
			0x00, 0x00, 0x1F, 0xFF,   // line 18 
			0x00, 0x00, 0x0F, 0xFF,   // line 19 
			0x80, 0x00, 0x0F, 0xFF,   // line 20 

			0x80, 0x00, 0x07, 0xFF,   // line 21 
			0x80, 0x00, 0x07, 0xFF,   // line 22 
			0xC0, 0x00, 0x07, 0xFF,   // line 23 
			0xC0, 0x00, 0x0F, 0xFF,   // line 24 

			0xE0, 0x00, 0x0F, 0xFF,   // line 25 
			0xF0, 0x00, 0x1F, 0xFF,   // line 26 
			0xF0, 0x00, 0x1F, 0xFF,   // line 27 
			0xF8, 0x00, 0x3F, 0xFF,   // line 28 

			0xFE, 0x00, 0x7F, 0xFF,   // line 29 
			0xFF, 0x00, 0xFF, 0xFF,   // line 30 
			0xFF, 0xC3, 0xFF, 0xFF,   // line 31 
			0xFF, 0xFF, 0xFF, 0xFF    // line 32 
	};

	// Yin-shaped cursor XOR mask 

	BYTE XORmaskCursor[] = 
	{ 
		0x00, 0x00, 0x00, 0x00,   // line 1 
			0x00, 0x03, 0xC0, 0x00,   // line 2 
			0x00, 0x3F, 0x00, 0x00,   // line 3 
			0x00, 0xFE, 0x00, 0x00,   // line 4 

			0x0E, 0xFC, 0x00, 0x00,   // line 5 
			0x07, 0xF8, 0x00, 0x00,   // line 6 
			0x07, 0xF8, 0x00, 0x00,   // line 7 
			0x0F, 0xF0, 0x00, 0x00,   // line 8 

			0x1F, 0xF0, 0x00, 0x00,   // line 9 
			0x1F, 0xE0, 0x00, 0x00,   // line 10 
			0x3F, 0xE0, 0x00, 0x00,   // line 11 
			0x3F, 0xE0, 0x00, 0x00,   // line 12 

			0x3F, 0xF0, 0x00, 0x00,   // line 13 
			0x7F, 0xF0, 0x00, 0x00,   // line 14 
			0x7F, 0xF8, 0x00, 0x00,   // line 15 
			0x7F, 0xFC, 0x00, 0x00,   // line 16 

			0x7F, 0xFF, 0x00, 0x00,   // line 17 
			0x7F, 0xFF, 0x80, 0x00,   // line 18 
			0x7F, 0xFF, 0xE0, 0x00,   // line 19 
			0x3F, 0xFF, 0xE0, 0x00,   // line 20 

			0x3F, 0xC7, 0xF0, 0x00,   // line 21 
			0x3F, 0x83, 0xF0, 0x00,   // line 22 
			0x1F, 0x83, 0xF0, 0x00,   // line 23 
			0x1F, 0x83, 0xE0, 0x00,   // line 24 

			0x0F, 0xC7, 0xE0, 0x00,   // line 25 
			0x07, 0xFF, 0xC0, 0x00,   // line 26 
			0x07, 0xFF, 0xC0, 0x00,   // line 27 
			0x01, 0xFF, 0x80, 0x00,   // line 28 

			0x00, 0xFF, 0x00, 0x00,   // line 29 
			0x00, 0x3C, 0x00, 0x00,   // line 30 
			0x00, 0x00, 0x00, 0x00,   // line 31 
			0x00, 0x00, 0x00, 0x00    // line 32 
	};
	char bitp[] = {0x00, 0x11, 0x55, 0x77, 0xff};

#define SETBB(mem,bitnum) ((mem)[(bitnum) >> 3] |= (1 << (7-(bitnum) & 7)))
	memset(andmask, 0, size);
	memset(xormask, 0, size);

	for (j=0; j<g_cursor_size; j++) {
		const U8 *pd = &pixdata[g_cursor_size*4*(g_cursor_size-1-j)];
		for (i=0; i<g_cursor_size; i++) {
			Color c = *(Color*)pd;
			pd+=4;
			if (c.a < 10) {
				// Screen
				SETBB(andmask, bitc);
				//SETBB(xormask, bitc); // Inverse
			} else {
				// Black -> white
				int ind = (c.r + c.g + c.b) * ARRAY_SIZE(bitp) / 766;
				assert(ind >= 0 && ind < ARRAY_SIZE(bitp));
				if (bitp[ind] & (1 << ((i + j) % 8))) {
					SETBB(xormask, bitc);
				}
			}
			bitc++;
		}
	}
	

	hCurs1 = CreateCursor( glob_hinstance,   // app. instance 
		hotspot_x,         // horizontal position of hot spot 
		hotspot_y,         // vertical position of hot spot 
		dwWidth,           // cursor width 
		dwHeight,          // cursor height 
		andmask,     // AND mask 
		xormask);   // XOR mask 


	return hCurs1;
}


// Most of this logic is from:
// http://support.microsoft.com/kb/318876
static HCURSOR CreateAlphaCursor(const U8 *pixdata,int hotspot_x,int hotspot_y)
{
	HDC				hdc;
	HBITMAP			hMonoBitmap;
	DWORD			dwWidth, dwHeight;
	DWORD			*lpdwPixel;
	HBITMAP			hBitmap;
	void			*lpBits;
	DWORD			x,y;
	HCURSOR			hAlphaCursor = NULL;
	BMAP			bmap;
	BITMAPV5HEADER	*bi;
	ICONINFO		ii;
	static U32 bitmask[64*64/32];
	int				num_entries;
	PALETTEENTRY	syspals[256];
    HDC				hdcfrom, hdcto;
    HBITMAP			hBitmap2, hbmfrom, hbmto;

	if (game_state.compatibleCursors)
		return CreateCompatibleCursor(pixdata, hotspot_x, hotspot_y);

	dwWidth = g_cursor_size; // width of cursor
	dwHeight = g_cursor_size; // height of cursor
	bi = &bmap.bi;
	ZeroMemory(bi,sizeof(BITMAPV5HEADER));

	bi->bV5Size = sizeof(BITMAPV5HEADER);
	bi->bV5Width = dwWidth;
	bi->bV5Height = dwHeight;
	bi->bV5Planes = 1;
	bi->bV5BitCount = 8;
	bi->bV5Compression = BI_RGB;

	// The following mask specification specifies a supported 32 BPP
	// alpha format for Windows XP.
	if (supportsGdiPlus())
	{
		bi->bV5BitCount = 32;
		bi->bV5Compression = BI_BITFIELDS;

		if (IsUsingCider())
		{
			// Cider always expects this
			bi->bV5RedMask   = 0x000000FF;
			bi->bV5GreenMask = 0x0000FF00;
			bi->bV5BlueMask  = 0x00FF0000;
			bi->bV5AlphaMask = 0xFF000000;
		}
		else
		{
			// GDI (non-Cider) wants this
			bi->bV5RedMask   = 0x00FF0000;
			bi->bV5GreenMask = 0x0000FF00;
			bi->bV5BlueMask  = 0x000000FF;
			bi->bV5AlphaMask = 0xFF000000;
		}
	}

	for(y=0;y<256;y++)
		bmap.palette[y] = y;
	hdc = GetDC(NULL);
	num_entries = GetSystemPaletteEntries(hdc,0,256,syspals);
	hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)bi, DIB_RGB_COLORS, (void **)&lpBits, 0, 0);
	ReleaseDC(NULL,hdc);

	// Set the alpha values for each pixel in the cursor so that
	// the complete cursor is semi-transparent.
	lpdwPixel = (DWORD *)lpBits;
	memset(bitmask,0,sizeof(bitmask));
	if (!supportsGdiPlus())
	{
		U8	*pix = (void*)lpBits,*mask = (U8*)bitmask;

		for (y=0;y<dwHeight;y++)
		{
			for (x=0;x<dwWidth;x++)
			{
				if (pixdata[3] < 1)
					mask[((dwHeight-1-y)*dwWidth + (x)) >> 3] |= 1 << (7-(x & 7)); // Had to figure this out by trial and error. Grr.
				*pix = findNearestIdx(pixdata,syspals,256);
				pix++;
				pixdata += 4;
			}
		}
	}
	else
	{
		if (IsUsingCider())
		{
			// GDI+ channel masking isn't supported under Cider.
			for (y=0;y<dwHeight;y++)
			{
				for (x=0;x<dwWidth;x++)
				{
					*lpdwPixel++ = (pixdata[2] << 16) | (pixdata[1] << 8) | (pixdata[0] << 0) | (pixdata[3] << 24);

					// Don't know if Cider needs this
					/*
					if (pixdata[3] < 1)
						mask[((dwHeight-1-y)*dwWidth + (x)) >> 3] |= 1 << (7-(x & 7)); // Had to figure this out by trial and error. Grr.
					*/

					pixdata += 4;
				}
			}
		}
		else
		{
			U8 *mask = (U8*)bitmask;
			for (y=0;y<dwHeight;y++)
			{
				for (x=0;x<dwWidth;x++)
				{
					*lpdwPixel++ = (pixdata[0] << 16) | (pixdata[1] << 8) | (pixdata[2] << 0) | (pixdata[3] << 24);

					// Not 100% sure we need this
					if (pixdata[3] < 1)
						mask[((dwHeight-1-y)*dwWidth + (x)) >> 3] |= 1 << (7-(x & 7)); // Had to figure this out by trial and error. Grr.

					pixdata += 4;
				}
			}
		}

		// TODO: Would be nice if we did not need to do the above reorder
		//memcpy(lpdwPixel, pixdata, dwWidth * dwHeight * 4);
	}
	hMonoBitmap = CreateBitmap(dwWidth,dwHeight,1,1,bitmask);

	ii.fIcon = FALSE; // Change fIcon to TRUE to create an alpha icon
	ii.xHotspot = hotspot_x;
	ii.yHotspot = hotspot_y;
	ii.hbmMask = hMonoBitmap;
	ii.hbmColor = hBitmap;

	// make a copy of bitmap for the current device
	hdc = GetDC(NULL);
	hdcfrom = CreateCompatibleDC(hdc);
	hdcto = CreateCompatibleDC(hdc);
	hBitmap2 = CreateCompatibleBitmap(hdc, dwWidth, dwHeight);
	hbmfrom = SelectObject(hdcfrom, hBitmap);
	hbmto = SelectObject(hdcto, hBitmap2);
	BitBlt(hdcto, 0, 0, dwWidth, dwHeight, hdcfrom, 0, 0, SRCCOPY);
	SelectObject(hdcfrom, hbmfrom);
	SelectObject(hdcto, hbmto);
	DeleteDC(hdcfrom);
	DeleteDC(hdcto);
	ReleaseDC(NULL, hdc);
	// should have a normal device-dependent bitmap now

	// Create the alpha cursor with the alpha DIB section
	hAlphaCursor = CreateIconIndirect(&ii);
	DeleteObject(hBitmap); 
	DeleteObject(hBitmap2);
	DeleteObject(hMonoBitmap); 
	return hAlphaCursor;
} 

static void initCursorPbuffer()
{
	PBufferFlags flags;
	//set flags in case of FBOs
	flags = PB_ALPHA|PB_COLOR_TEXTURE|PB_DEPTH_TEXTURE;
	if(rdr_caps.supports_fbos)
		flags |= PB_FBO;
	pbufInit(&cursor_pbuf,g_cursor_size,g_cursor_size,flags,0,0,0,0,0,0);
}

void hwcursorInit()
{
	// Windows Vista seems unable to draw 64x64 cursors in fullscreen mode. The problem does
	//  not appear in XP or Windows 7. See COH-14205.
	if (game_state.compatibleCursors || (IsUsingVistaOrLater() && !IsUsingWin7orLater()))
		g_cursor_size = 32;
	else if (supportsGdiPlus())
		g_cursor_size = 64;

#ifdef CURSOR_SWBLENDING
	assert(g_cursor_size <= MAX_CURSOR_SIZE);
#else
	initCursorPbuffer();
#endif
}

void hwcursorSet()
{
	PERFINFO_AUTO_START("hwcursorSet", 1);
		if (curse && game_state.can_set_cursor)
			SetCursor(curse);
		else if(GetCursor())
			SetCursor(GetCursor());
	PERFINFO_AUTO_STOP();
}

void hwcursorReload(HCURSOR* phCursor, int hotspot_x,int hotspot_y)
{
	
	U8		*pixdata;

	if(!phCursor || !*phCursor)
	{
#ifdef CURSOR_SWBLENDING
		pixdata = cursor_pixmap;
#else
		U8 buffer[32*1024];
		pixdata = pbufFrameGrab(&cursor_pbuf, buffer, sizeof(buffer));
#endif

		if(!pixdata)
			return;

		if (curse && !curseNoDestroy)
		{
			//SetCursor(0);
			DestroyIcon(curse);
		}
		curse = CreateAlphaCursor(pixdata,hotspot_x,hotspot_y);
#ifdef CURSOR_SWBLENDING
#else
		if(pixdata != buffer)
			free(pixdata);
#endif
			
		if(phCursor)
		{
			*phCursor = curse;
			curseNoDestroy = 1;
		}
		else
		{
			curseNoDestroy = 0;
		}
	}
	else
	{
		curse = *phCursor;
		curseNoDestroy = 1;
	}
	
	hwcursorSet();
}

void hwcursorReloadDefault()
{
	static int init;
	static AtlasTex *cursor;

	if (!init)
	{
		cursor = atlasLoadTexture("btest_cursor");
		init = 1;
	}
	iconIsNew( 0, 0 ); //Clears out statemanager.
	iconIsNew( cursor, 0 );
	hwcursorClear();
 	hwcursorBlit(cursor,cursor->width,cursor->height,0,0,1.0,0xffffffff);
	hwcursorReload(0,0,0);
}

#if 0
Hi Bruce,
 
Below is some code we dragged up. It's unpublished and under the DirectX Beta NDA so please don't redistribute.
 To create an alpha blended cursor or icon, you need to create a DIB section which contains alpha values and then call CreateIconIndirect with that DIB section. Although the mask bitmap in the ICONINFO structure is not necessary, you need to create an empty monochrome bitmap to send to CreateIconIndirect as the mask bitmap. The alpha bits for each pixel in the DIB section bitmap will determine the visibility for each pixel in the cursor or icon that is created.

The following procedure should be used to create an alpha blended cursor or icon:

1) Define a 32 BPP alpha blended DIB by filling out a BITMAPV5HEADER structure as in the example code below. 
2) Create a DIB section based off of that BITMAPV5HEADER by calling CreateDIBSection. 
3) Fill the DIB section with the bitmap information and alpha information desired for your alpha blended cursor or icon. 
4) Fill out an ICONINFO structure. Place an empty monochrome bitmap in the hbmMask field and place the alpha blended DIB section in the hbmColor field. 
5) Finally, call CreateIconIndirect to create the cursor or icon. 

The following C++ code demonstrates how to create an alpha blended cursor. The same code can be used to create an alpha blended icon by changing the fIcon member of the ICONINFO structure to TRUE:
#endif
