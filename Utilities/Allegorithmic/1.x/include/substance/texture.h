/** @file texture.h
	@brief Platform specific texture (results) structure
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceTexture structure. Platform dependent definition. Used
	to return resulting output textures. */

#ifndef _SUBSTANCE_TEXTURE_H
#define _SUBSTANCE_TEXTURE_H


/** Platform dependent definitions */
#include "platformdep.h"



/* Platform specific organization */
#ifdef SUBSTANCE_PLATFORM_XBOX360
	#define SUBSTANCE_TEXTURE_D3D9 1
#endif /* ifdef SUBSTANCE_PLATFORM_XBOX360 */
#ifdef SUBSTANCE_PLATFORM_D3D9PC
	#define SUBSTANCE_TEXTURE_D3D9 1
#endif /* ifdef SUBSTANCE_PLATFORM_D3D9PC */

#ifdef SUBSTANCE_PLATFORM_ORCA
	#define SUBSTANCE_TEXTURE_OGLFRAMEBUFFER 1
#endif /* ifdef SUBSTANCE_PLATFORM_ORCA */
#ifdef SUBSTANCE_PLATFORM_OGL2
	#define SUBSTANCE_TEXTURE_OGLFRAMEBUFFER 1
#endif /* ifdef SUBSTANCE_PLATFORM_OGL2 */
#ifdef SUBSTANCE_PLATFORM_OGLCG
	#define SUBSTANCE_TEXTURE_OGLFRAMEBUFFER 1
#endif /* ifdef SUBSTANCE_PLATFORM_OGLCG */
#ifdef SUBSTANCE_PLATFORM_PS3PSGL
	#define SUBSTANCE_TEXTURE_OGLFRAMEBUFFER 1
#endif /* ifdef SUBSTANCE_PLATFORM_PS3PSGL */
#ifdef SUBSTANCE_PLATFORM_PS3GCM
	#define SUBSTANCE_TEXTURE_GCM
#endif /* ifdef SUBSTANCE_PLATFORM_PS3PSGL */


#if defined(SUBSTANCE_TEXTURE_D3D9)

	/** @brief Substance engine texture (results) structure

	    Direct3D9 PC and XBox 360 version. */
	typedef struct
	{
		/** @brief Direct3D Texture handle */
		LPDIRECT3DTEXTURE9 handle;
		
	} SubstanceTexture;
	
#elif defined(SUBSTANCE_TEXTURE_OGLFRAMEBUFFER)
	
	/** @brief Substance engine texture (results) structure

	    OpenGL w/ framebuffer version. */
	typedef struct
	{
		/** @brief OpenGL texture name */
		unsigned int textureName;
		
		/** @brief OpenGL framebuffer object name */
		unsigned int fboName;
		
	} SubstanceTexture;	

#elif defined(SUBSTANCE_PLATFORM_D3D10PC)

	/** @brief Substance engine texture (results) structure

	    Direct3D10 PC version. */
	typedef struct
	{
		/** @brief Direct3D Texture handle */
		ID3D10Texture2D* handle;
		
	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_PS3GCM)
	
	/** @brief Substance engine texture (results) structure

	    PS3 GCM version. */
	typedef struct
	{
		/** @brief Pointer to the result array */
		void *buffer;
		
		/** @brief GCM texture description. */
		CellGcmTexture gcmTexture;
		
	} SubstanceTexture;		

#elif defined(SUBSTANCE_PLATFORM_BLEND)
	
	/** @brief Substance engine texture (results) structure

	    Blend platform. Result in system memory. */
	typedef struct
	{
		/** @brief Pointer to the result array
			
			This value is NULL if the buffer of this output has been allocated
			by the SubstanceCallbackOutputMalloc callback. See callbacks.h for
			further information.
			
			If present, the buffer contains all the mipmap levels concatenated
			without any padding. If a different memory layout is required, use
			the SubstanceCallbackOutputMalloc callback. */
		void *buffer;
		
		/** @brief Channels order in memory
	
			One of the SubstanceChannelsOrder enum (defined in pixelformat.h).

			The default channel order  depends on the format and on the 
			Substance engine concrete implementation (CPU/GPU, GPU API, etc.).
			See the platform specific documentation. */
		unsigned char channelsOrder;
		
	} SubstanceTexture;	

#else

	/** @todo Add other APIs texture definition */
	#error NYI
	
#endif



#endif /* ifndef _SUBSTANCE_TEXTURE_H */
