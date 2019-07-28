/** @file blend_ddssave.c
    @brief DDS writing definition
    @author Christophe Soum
    @date 20080915
    @copyright Allegorithmic. All rights reserved.
	
	Allows to save on the disk in DDS format (RGBA and/or DXT).
*/


#ifndef _SUBSTANCE_TUTORIALS_BLEND_COMMON_BLEND_DDSSAVE_H
#define _SUBSTANCE_TUTORIALS_BLEND_COMMON_BLEND_DDSSAVE_H

/* Substance include */
#include <substance/substance.h>


/** @brief Write a DDS file from Substance texture
	@param resultTexture The resulting texture
	@param desc Description of the texture 
	@param cmp Texture index (used for filename) */
void algImgIoDDSWriteFile(
	SubstanceTexture *resultTexture,
	const SubstanceOutputDesc *desc,
	unsigned int cmp);	

#endif /* ifndef _SUBSTANCE_TUTORIALS_BLEND_COMMON_BLEND_DDSSAVE_H */
