/** @file blend_ddssave.c
    @brief DDS writing implementation
    @author Christophe Soum
    @date 20080915
    @copyright Allegorithmic. All rights reserved.
	
	Allows to save on the disk in DDS format (RGBA and/or DXT).
*/


#include "blend_ddssave.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/** @brief DDS formats enumeration */
typedef enum 
{
	ALG_IMGIO_DDSFMT_ABGR    = 0x1,
	ALG_IMGIO_DDSFMT_ARGB    = 0x3,
	ALG_IMGIO_DDSFMT_XBGR    = 0x0,
	ALG_IMGIO_DDSFMT_XRGB    = 0x2,
	ALG_IMGIO_DDSFMT_DXT     = 0x8,
	ALG_IMGIO_DDSFMT_L       = 0x4,
	ALG_IMGIO_DDSFMT_RGBA16  = 0x11,
	ALG_IMGIO_DDSFMT_L16     = 0x14,
	ALG_IMGIO_DDSFMT_RGBA16F = 0x51,
	ALG_IMGIO_DDSFMT_L16F    = 0x54,
} AlgImgIoDDSFormat;

/** @brief Size of the DDS header */
#define ALG_IMGIO_DDSHEADERSIZE 128


/** @brief Fill a DDS header from format/size informations
	@param width The level 0 (base) width
	@param height The level 0 (base) height
	@param pitch Pitch of the level 0 (size of a line of BLOCKS for compress 
		formats). Cannot be ==0.
	@param mips Number of mips (1: no mips). Cannot be ==0.
	@param format One of the AlgImgIoDDSFormat enum
	@param compression Only useful if format==ALG_IMGIO_DDSFMT_DXT. DXT format
		(1,2,3,4,5,6==DXTn).
	@param header Pointer on a array of ALG_IMGIO_DDSHEADERSIZE size that will
		receive the result */
void algImgIoDDSFillHeader(
	unsigned int width,
	unsigned int height,
	size_t pitch,
	unsigned int mips,
	AlgImgIoDDSFormat format,
	unsigned int compression,
	unsigned char *header);


/** @brief Write a DDS file from Substance texture
	@param resultTexture The resulting texture
	@param desc Description of the texture 
	@param cmp Texture index (used for filename) */
void algImgIoDDSWriteFile(
	SubstanceTexture *resultTexture,
	const SubstanceOutputDesc *desc,
	unsigned int cmp)	
{
	char nameout[512];
	FILE *f;
	unsigned int width,height,blockw,blockh,k;
	unsigned char ddsheader[ALG_IMGIO_DDSHEADERSIZE];
	AlgImgIoDDSFormat ddsformat = ALG_IMGIO_DDSFMT_XBGR;
	unsigned int ddscompression = 0;
	size_t ddsblocksize = 4, ddspitch = 0, ddssize = 0;
	unsigned int mips = desc->mipmapCount;
	
	blockw=width=desc->level0Width;
	blockh=height=desc->level0Height;
	
	sprintf(nameout,"out_%04u_%08X.dds",cmp,desc->outputId);
	f = fopen(nameout, "wb");
	
	/* Find format */
	switch (desc->pixelFormat&~Substance_PF_sRGB)
	{
		case Substance_PF_RGBA|Substance_PF_16b:
		{
			ddssize = (ddspitch = (ddsblocksize=8) * width) * height;
			ddsformat = ALG_IMGIO_DDSFMT_RGBA16;
		}
		break;
			
		case Substance_PF_L|Substance_PF_16b:
		{
			ddssize = (ddspitch = (ddsblocksize=2) * width) * height;
			ddsformat = ALG_IMGIO_DDSFMT_L16;
		}
		break;
		
		case Substance_PF_RGBA:
			ddsformat = 0x1;
		case Substance_PF_RGBx:
		{
			ddspitch = ddsblocksize * width;
			ddssize = ddspitch * height;
			
			switch (resultTexture->channelsOrder)
			{
				case Substance_ChanOrder_RGBA: break;
				case Substance_ChanOrder_BGRA: ddsformat |= 0x2; break;
				default: 
				{
					printf("Output %d: non-supported channel order %d.\n",
						cmp,resultTexture->channelsOrder);
				}
				return; 
			}
		}
		break;
		
		case Substance_PF_DXT2:
		case Substance_PF_DXT3:
		case Substance_PF_DXT4:
		case Substance_PF_DXT5:
			ddsblocksize *= 2;
		case Substance_PF_DXT1:
		{
			ddsblocksize *= 2;
			ddspitch = ddsblocksize * (blockw=max(1,width/4));
			ddssize = ddspitch * (blockh=max(1,height/4));
			ddsformat = ALG_IMGIO_DDSFMT_DXT;
			ddscompression = ((desc->pixelFormat&0x1C)>>2) + 1;
		}
		break;
		
		default: 
		{
			printf("Output %d: non-supported pixelformat %d.\n",
				cmp,desc->pixelFormat);
		}
		return; 
	}
	
	/* Compute mips size */
	if (mips==0)
	{
		unsigned int tx = max(width,height)>>1;
		mips = 1;
		for (;tx;tx=tx>>1,++mips); 
	}
	for (k=1;k<mips;++k)
	{
		ddssize += ddsblocksize * (max(1,(blockw>>k))) * 
			(max(1,(blockh>>k)));
	}
	
	/* Save DDS file */
	algImgIoDDSFillHeader(
		width,
		height,
		ddspitch,
		mips,
		ddsformat,
		ddscompression,
		ddsheader);
	
	fwrite((const char*)ddsheader,ALG_IMGIO_DDSHEADERSIZE,1,f);
	fwrite(resultTexture->buffer,ddssize,1,f);
	
	fclose(f);	
}	
	
	
#define ALG_IMGIO_DDS_WRITEUINT32(array,startindex,val) \
	(header[(startindex)+3]=(unsigned char)((val)>>24), \
	header[(startindex)+2]=(unsigned char)(((val)>>16)&0xFF), \
	header[(startindex)+1]=(unsigned char)(((val)>>8)&0xFF), \
	header[(startindex)]=(unsigned char)((val)&0xFF))
	
	
	
/** @brief Fill a DDS header from format/size informations
	@param width The level 0 (base) width
	@param height The level 0 (base) height
	@param pitch Pitch of the level 0 (size of a line of BLOCKS for compress 
		formats). Cannot be ==0.
	@param mips Number of mips (1: no mips). Cannot be ==0.
	@param format One of the AlgImgIoDDSFormat enum
	@param compression Only useful if format==ALG_IMGIO_DDSFMT_DXT. DXT format
		(1,2,3,4,5,6==DXTn).
	@param header Pointer on a array of ALG_IMGIO_DDSHEADERSIZE size that will
		receive the result */
void algImgIoDDSFillHeader(
	unsigned int width,
	unsigned int height,
	size_t pitch,
	unsigned int mips,
	AlgImgIoDDSFormat format,
	unsigned int compression,
	unsigned char *header)
{
	unsigned int v;
	unsigned int isRgb = (ALG_IMGIO_DDSFMT_DXT!=format);

	header[0]='D';
	header[1]='D';
	header[2]='S';
	header[3]=' ';                                /* DDS tag */
	
	ALG_IMGIO_DDS_WRITEUINT32(header,4,0x7c);     /* Header size */
	
	v = 0x1007 | (isRgb?0x8:0x80000) | (mips>1?0x20000:0);

	if(ALG_IMGIO_DDSFMT_L16F==format||ALG_IMGIO_DDSFMT_RGBA16F==format)
	{
		v = 0x1007;
	}

	ALG_IMGIO_DDS_WRITEUINT32(header,8,v);        /* Flags */
	ALG_IMGIO_DDS_WRITEUINT32(header,12,height);  /* Height */
	ALG_IMGIO_DDS_WRITEUINT32(header,16,width);   /* Width */
	v = isRgb ? pitch : (pitch*max(1,height/4));
	ALG_IMGIO_DDS_WRITEUINT32(header,20,v);       /* Pitch/LinearSize */
	ALG_IMGIO_DDS_WRITEUINT32(header,24,0);       /* :pad: */
	v = mips>1 ? mips : 0;
	ALG_IMGIO_DDS_WRITEUINT32(header,28,v);       /* Mips */
	
	memset(&header[32],0,128-32);                 /* Fill other fields with 0 */
	
	ALG_IMGIO_DDS_WRITEUINT32(header,76,0x20);    /* Pixel format size */
	v = 0x4;
	switch (format)
	{
		case ALG_IMGIO_DDSFMT_ABGR:
		case ALG_IMGIO_DDSFMT_ARGB: v = 0x41; break;
		case ALG_IMGIO_DDSFMT_XBGR:
		case ALG_IMGIO_DDSFMT_XRGB: v = 0x40; break;
		case ALG_IMGIO_DDSFMT_L:
		case ALG_IMGIO_DDSFMT_L16: v = 0x20000; break;
	}
	ALG_IMGIO_DDS_WRITEUINT32(header,80,v);       /* Pixel format flags */
	
	switch (format)
	{
		case ALG_IMGIO_DDSFMT_ABGR:
		case ALG_IMGIO_DDSFMT_ARGB:
		case ALG_IMGIO_DDSFMT_XBGR:
		case ALG_IMGIO_DDSFMT_XRGB:
		{
			/* RAW pixel format desc. */
			ALG_IMGIO_DDS_WRITEUINT32(header,88,32);  /* Number of bits */
			v = format&0x2 ? 0x00ff0000 : 0x000000ff ;
			ALG_IMGIO_DDS_WRITEUINT32(header,92,v);
			ALG_IMGIO_DDS_WRITEUINT32(header,96,0x0000ff00);
			v = format&0x2 ? 0x000000ff : 0x00ff0000 ;
			ALG_IMGIO_DDS_WRITEUINT32(header,100,v);
			v = format&0x1 ? 0xff000000 : 0 ;
			ALG_IMGIO_DDS_WRITEUINT32(header,104,v);
		}
		break;
	
		case ALG_IMGIO_DDSFMT_DXT:
		{
			/* FourCC */
			header[84]='D';
			header[85]='X';
			header[86]='T';
			header[87]='1'+((unsigned char)compression-1);
		}
		break;
		
		case ALG_IMGIO_DDSFMT_RGBA16:
		{
			/* FourCC */
			header[84]=0x24;
		}
		break;
		
		case ALG_IMGIO_DDSFMT_L:
		case ALG_IMGIO_DDSFMT_L16:
		{
			v = format&0x10 ? 0x10 : 0x8;
			ALG_IMGIO_DDS_WRITEUINT32(header,88,v);
			v = format&0x10 ? 0xFFFF : 0xFF;
			ALG_IMGIO_DDS_WRITEUINT32(header,92,v);
		}
		break;
		case ALG_IMGIO_DDSFMT_L16F:
		{
			//four cc code for this format
			v = 0x6f;
			ALG_IMGIO_DDS_WRITEUINT32(header,84,v);
		}
        break;
        case ALG_IMGIO_DDSFMT_RGBA16F:
        {
            v = 0x71;
			ALG_IMGIO_DDS_WRITEUINT32(header,84,v);
        }
	};
	
	v = 0x1000 | (mips>1?0x400008:0);
	ALG_IMGIO_DDS_WRITEUINT32(header,108,v);      /* Caps */
}

