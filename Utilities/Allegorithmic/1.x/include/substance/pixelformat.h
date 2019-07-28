/** @file pixelformat.h
	@brief Substance pixel formats definitions. Use by texture outputs
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstancePixelFormat and SubstanceChannelsOrder enumerations.
	
*/

#ifndef _SUBSTANCE_PIXELFORMAT_H
#define _SUBSTANCE_PIXELFORMAT_H


/** @brief Substance pixel formats enumeration

	A pixel format is stored on 8 bits:
	  - 2bits (LSB): RAW/DXT/other format.
	  - 3bits: sub-format index (e.g. DXT1/2/3/. etc.).
	  - 3bits (MSB): other flags (sRGB, etc.)  */
typedef enum
{
	/* Format (2 bits) */
	Substance_PF_RAW  = 0x0,                  /**< Non-compressed flag */
	Substance_PF_DXT  = 0x1,                  /**< DXT compression flag */
	/* etc. */
	
	/* RAW formats (3bits) */
	Substance_PF_RGBA = Substance_PF_RAW | 0x0,  /**< RGBA 4x8bits format */
	Substance_PF_RGBx = Substance_PF_RAW | 0x4,  /**< RGBx (3+1:pad)x8bits format */
	Substance_PF_RGB  = Substance_PF_RAW | 0x8,  /**< RGB 3x8bits format */
	Substance_PF_L    = Substance_PF_RAW | 0xC,  /**< Luminance 8bits format */
	
	Substance_PF_16b  = Substance_PF_RAW | 0x10, /**< 16bits flag */
	
	/* DXT formats (3bits) */
	Substance_PF_DXT1 = Substance_PF_DXT | 0x0,  /**< DXT1 format */
	Substance_PF_DXT2 = Substance_PF_DXT | 0x4,  /**< DXT2 format */
	Substance_PF_DXT3 = Substance_PF_DXT | 0x8,  /**< DXT3 format */
	Substance_PF_DXT4 = Substance_PF_DXT | 0xC,  /**< DXT4 format */
	Substance_PF_DXT5 = Substance_PF_DXT | 0x10, /**< DXT5 format */
	Substance_PF_DXTn = Substance_PF_DXT | 0x14, /**< DXTn format */
	
	/* Other flags */
	Substance_PF_sRGB = 0x20,                 /**< sRGB (gamma trans.) flag */
	/* etc. */

} SubstancePixelFormat;


/** @brief Substance channels order in memory enumeration

	Useful on generic platform that put results in system mem (BLEND).
	8 bits format.
	| 2bits: channel A index | 2bits: channel B index |
	  2bits: channel G index | 2bits: channel R index | */
typedef enum
{
	Substance_ChanOrder_NC     = 0,     /**< Not applicable */
	Substance_ChanOrder_RGBA   = 0xE4,
	Substance_ChanOrder_BGRA   = 0xC6,
	Substance_ChanOrder_ABGR   = 0x1B,
	Substance_ChanOrder_ARGB   = 0x39

} SubstanceChannelsOrder;


#endif /* ifndef _SUBSTANCE_PIXELFORMAT_H */
