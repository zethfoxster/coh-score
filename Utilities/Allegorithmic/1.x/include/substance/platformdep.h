/** @file platformdep.h
	@brief Platform specific definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines platform specific macros. One of these macros must already be
	defined (preprocessor #define):
	  - SUBSTANCE_PLATFORM_BLEND: generic platform, results in system memory
	  - SUBSTANCE_PLATFORM_D3D9PC: PC Direct3D9 API
	  - SUBSTANCE_PLATFORM_XBOX360: Xbox 360
	  - SUBSTANCE_PLATFORM_D3D10PC: PC Direct3D10 API
	  - SUBSTANCE_PLATFORM_OGL2: OpenGL 2 GLSL API
	  - SUBSTANCE_PLATFORM_OGLCG: OpenGL Cg API
	  - SUBSTANCE_PLATFORM_PS3PSGL: PSGL (OpenGL ES) PS3 API
	  - SUBSTANCE_PLATFORM_PS3GCM: GCM PS3 API
	  etc. */

#ifndef _SUBSTANCE_PLATFORMDEP_H
#define _SUBSTANCE_PLATFORMDEP_H

#ifdef __WIN32__
	#define SUBSTANCE_WIN32 1
#endif
#ifdef _WIN32
	#define SUBSTANCE_WIN32 1
#endif


/* Platform specific headers */

#if defined(SUBSTANCE_PLATFORM_XBOX360)
	
	/* 	Xbox 360 */
	#include <xtl.h>
		
#elif defined(SUBSTANCE_PLATFORM_D3D9PC)

	/* PC Direct3D9 API */
	#ifndef D3D_DEBUG_INFO
		#define D3D_DEBUG_INFO 1
	#endif //D3D_DEBUG_INFO
	#pragma pack(push,8)
	#define D3D_OVERLOADS 1
	#include <d3d9.h>
	#pragma pack(pop)

#elif defined(SUBSTANCE_PLATFORM_D3D10PC)

	/* PC Direct3D10 API */
	#include <d3d10.h>
	
#elif defined(SUBSTANCE_PLATFORM_ORCA)

	/* ORCA platform */
	#include "definesorca.h"

#elif defined(SUBSTANCE_PLATFORM_OGL2)

	/* OpenGL GLSL platform */
	/* No header needed */
	
#elif defined(SUBSTANCE_PLATFORM_OGLCG)

	/* OpenGL Cg platform */
	/* No header needed */
	
#elif defined(SUBSTANCE_PLATFORM_PS3PSGL)

	/* PSGL PS3 platform */
	/* No header needed */

#elif defined(SUBSTANCE_PLATFORM_PS3GCM)

	/* GCM PS3 platform */
	#include <cell/gcm.h>
	
#elif defined(SUBSTANCE_PLATFORM_BLEND)
	
	/* Blend platform */
	/* No header needed */
	
#else

	/* platform not supported / define not set */
	#error Platform NYI or proper SUBSTANCE_PLATFORM_xxx preprocessor macro not set
	
#endif


/* Base types defines */
#include <stddef.h>

/* C/C++ macros */
#ifdef __cplusplus
	#define SUBSTANCE_EXTERNC extern "C"
#else
	#define SUBSTANCE_EXTERNC
#endif

/* SUBSTANCE_EXPORT macros */
#ifdef SUBSTANCE_DLL
	#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC __declspec(dllexport)
#else
	#define SUBSTANCE_EXPORT SUBSTANCE_EXTERNC
#endif

/* Callback functions */
#ifdef _MSC_VER
	#define SUBSTANCE_CALLBACK __stdcall
#else
	#define SUBSTANCE_CALLBACK
#endif


#endif /* ifndef _SUBSTANCE_PLATFORMDEP_H */
