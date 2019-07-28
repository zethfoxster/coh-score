#ifndef _TEX_GEN_H
#define _TEX_GEN_H

#include "stdtypes.h"
typedef struct BasicTexture BasicTexture;
typedef struct TexReadInfo TexReadInfo;

// pixel_format must be GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, or GL_COMPRESSED_RGBA_S3TC_DXT5_EXT

BasicTexture *texGenNew(int width,int height, char *name);
void texGenFree(BasicTexture *bind);
void texGenUpdate(BasicTexture *bind, U8 *bitmap);
void texGenUpdateEx(BasicTexture *bind, U8 *bitmap, int pixel_format, int mipcount);
void texGenUpdateLogical(BasicTexture *bind, U8 *bitmap, int pixel_format, int mipcount);
void texGenUpdateFromScreen(BasicTexture *bind, int x, int y, int width, int height, bool depth_buffer);
void texGenUpdateRegion(BasicTexture *bind, U8 *bitmap, int x, int y, int width, int height, int pixel_format);
void texGenUpdateRegionFromReadInfo(BasicTexture *bind, TexReadInfo *info, int x, int y, int width, int height);

int getImageByteCount(int pixel_format, int width, int height);
int getImageByteCountWithMips(int pixel_format, int width, int height, int mipcount);

#endif
