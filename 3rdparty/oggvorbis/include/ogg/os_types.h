/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os_types.h 23230 2005-09-07 20:08:00Z martin $

 ********************************************************************/
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */

#ifdef OV_STATICLIB
	#define _ogg_malloc(x)		cryptic_ogg_malloc(x, __FILE__, __LINE__)
	#define _ogg_calloc(x,y)	cryptic_ogg_calloc(x, y, __FILE__, __LINE__)
	#define _ogg_realloc(p,x)	cryptic_ogg_realloc(p, x, __FILE__, __LINE__)
	#define _ogg_free(x)		cryptic_ogg_free(x, __FILE__, __LINE__)

	void*	cryptic_ogg_malloc(int size, const char* fileName, int line);
	void*	cryptic_ogg_calloc(int num, int size, const char* fileName, int line);
	void*	cryptic_ogg_realloc(void *userData, int newSize, const char* fileName, int line);
	void	cryptic_ogg_free(void *userData, const char* fileName, int line);
#else
	#define _ogg_malloc(x)		malloc(x)
	#define _ogg_calloc(x,y)	calloc(x,y)
	#define _ogg_realloc(p,x)	realloc(p, x)
	#define _ogg_free(x)		free(x)
#endif

#if defined(_WIN32) 

#  if defined(__CYGWIN__)
#    include <_G_config.h>
     typedef _G_int64_t ogg_int64_t;
     typedef _G_int32_t ogg_int32_t;
     typedef _G_uint32_t ogg_uint32_t;
     typedef _G_int16_t ogg_int16_t;
     typedef _G_uint16_t ogg_uint16_t;
#  elif defined(__MINGW32__)
     typedef short ogg_int16_t;                                                                             
     typedef unsigned short ogg_uint16_t;                                                                   
     typedef int ogg_int32_t;                                                                               
     typedef unsigned int ogg_uint32_t;                                                                     
     typedef long long ogg_int64_t;                                                                         
     typedef unsigned long long ogg_uint64_t;  
#  elif defined(__MWERKS__)
     typedef long long ogg_int64_t;
     typedef int ogg_int32_t;
     typedef unsigned int ogg_uint32_t;
     typedef short ogg_int16_t;
     typedef unsigned short ogg_uint16_t;
#  else
     /* MSVC/Borland */
     typedef __int64 ogg_int64_t;
     typedef __int32 ogg_int32_t;
     typedef unsigned __int32 ogg_uint32_t;
     typedef __int16 ogg_int16_t;
     typedef unsigned __int16 ogg_uint16_t;
#  endif

#elif defined(__MACOS__)

#  include <sys/types.h>
   typedef SInt16 ogg_int16_t;
   typedef UInt16 ogg_uint16_t;
   typedef SInt32 ogg_int32_t;
   typedef UInt32 ogg_uint32_t;
   typedef SInt64 ogg_int64_t;

#elif defined(__MACOSX__) /* MacOS X Framework build */

#  include <sys/types.h>
   typedef int16_t ogg_int16_t;
   typedef u_int16_t ogg_uint16_t;
   typedef int32_t ogg_int32_t;
   typedef u_int32_t ogg_uint32_t;
   typedef int64_t ogg_int64_t;

#elif defined(__BEOS__)

   /* Be */
#  include <inttypes.h>
   typedef int16_t ogg_int16_t;
   typedef u_int16_t ogg_uint16_t;
   typedef int32_t ogg_int32_t;
   typedef u_int32_t ogg_uint32_t;
   typedef int64_t ogg_int64_t;

#elif defined (__EMX__)

   /* OS/2 GCC */
   typedef short ogg_int16_t;
   typedef unsigned short ogg_uint16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef long long ogg_int64_t;

#elif defined (DJGPP)

   /* DJGPP */
   typedef short ogg_int16_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef long long ogg_int64_t;

#elif defined(R5900)

   /* PS2 EE */
   typedef long ogg_int64_t;
   typedef int ogg_int32_t;
   typedef unsigned ogg_uint32_t;
   typedef short ogg_int16_t;

#elif defined(__SYMBIAN32__)

   /* Symbian GCC */
   typedef signed short ogg_int16_t;
   typedef unsigned short ogg_uint16_t;
   typedef signed int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef long long int ogg_int64_t;

#else

#  include <sys/types.h>
#  include <ogg/config_types.h>

#endif

#endif  /* _OS_TYPES_H */
