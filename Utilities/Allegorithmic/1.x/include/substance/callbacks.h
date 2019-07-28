/** @file callbacks.h
	@brief Substance callbacks signatures and related defines
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines callback function signatures used by handle and context structures.
*/

#ifndef _SUBSTANCE_CALLBACKS_H
#define _SUBSTANCE_CALLBACKS_H

/** Handle structure and related functions definitions pre-declaration */
struct SubstanceHandle_;

/** Platform dependent definitions */
#include "platformdep.h"


/** @brief Callback types enumeration

	This enumeration is used to name callbacks in the Substance context
	structure.*/
typedef enum
{
	Substance_Callback_Progress,
	Substance_Callback_MallocEx,
	Substance_Callback_OutputCompleted,
	Substance_Callback_OutOfMemory,
	Substance_Callback_JobCompleted,
	Substance_Callback_Malloc,
	Substance_Callback_Free,
	Substance_Callback_FreeEx,
	Substance_Callback_OutputMalloc,
	/* etc. */

	Substance_Callback_Internal_Count

} SubstanceCallbackEnum;


/** @brief MallocEx Callback flags enumeration

	This enumeration is used by the Substance_Callback_MallocEx callback.
	Specifies the platform memory type to allocate (exclusive flags). */
typedef enum
{
	Substance_MallocEx_MemTypeSystem  = 0x00,  /**< system memory type (dflt.)*/
	Substance_MallocEx_MemTypeVideo   = 0x10,  /**< video memory type */
	Substance_MallocEx_MemTypeMapped  = 0x20   /**< mapped main memory type */

} SubstanceMallocExFlag;


/* Begin of the EXTERNC block (if necessary) */
#ifdef __cplusplus
SUBSTANCE_EXTERNC {
#endif /* ifdef __cplusplus */


/** @typedef void SubstanceCallbackProgress
	@brief Type of the progress callback function

	@code void (*)(SubstanceHandle* handle,unsigned int progress,
		unsigned int progressMax) @endcode

	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'progress' parameter varies between 0 and progressMax-1. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackProgress)(
	struct SubstanceHandle_ *handle,
	unsigned int progress,
	unsigned int progressMax);


/** @typedef void SubstanceCallbackOutputCompleted
	@brief Type of the 'result output texture completed' callback function

	@code void (*)(SubstanceHandle* handle,unsigned int outputIndex,
		size_t jobUserData) @endcode

	This callback is called every time an output texture has been rendered.<BR>
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'outputIndex' parameter is the index of the completed output.<BR>
	'jobUserData' is the user information of the job corresponding to this
	output. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackOutputCompleted)(
	struct SubstanceHandle_ *handle,
	unsigned int outputIndex,
	size_t jobUserData);


/** @typedef void SubstanceCallbackOutOfMemory
	@brief Type of the 'out of memory' callback function

	@code void (*)(SubstanceHandle* handle,int memoryType) @endcode

	This callback is called when Substance runs out of memory.
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'memoryType' parameter is the memory concerned: -1: system memory, 0:
	video memory of the first GPU, 1: video memory of the second GPU, etc. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackOutOfMemory)(
	struct SubstanceHandle_ *handle,
	int memoryType);


/** @typedef void SubstanceCallbackJobCompleted
	@brief Type of the 'render job completed' callback function

	@code void (*)(SubstanceHandle* handle,size_t jobUserData) @endcode

	This callback is called when Substance finishes a job and is about to
	start the next job in the render list.<BR>
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	'jobUserData' is the integer/pointer set by the user at the creation of
	this job. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackJobCompleted)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData);


/**	@typedef void* SubstanceCallbackMalloc
	@brief Type of the 'system memory allocation' callback function

	@code void* (*)(unsigned int bytesCount,unsigned int alignment) @endcode

	This callback allows to use user-defined dynamic memory allocation.<BR>
	The 'bytesCount' parameter is the number of bytes to allocate.<BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer. */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackMalloc)(
	unsigned int bytesCount,
	unsigned int alignment);


/** @typedef void SubstanceCallbackFree
	@brief Type of the 'system memory de-allocation' callback function

	@code void (*)(void* bufferPtr) @endcode

	This callback allows to free buffers allocated with the
	SubstanceCallbackMalloc callback.<BR>
	The 'bufferPtr' parameter is the buffer to free. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackFree)(
	void* bufferPtr);

	
/** @typedef void* SubstanceCallbackMallocEx
	@brief Type of the 'extended system memory allocation' callback function
	@warning This callback must be implemented on some platforms. Please see 
		the platform specific Substance documentation.

	@code void* (*)(
		unsigned int flags,
		unsigned int bytesCount,
		unsigned int alignment) @endcode

	This callback extend the SubstanceCallbackMalloc callback. In addition, 
	it allows the allocation of platform specific memory buffer types.

	The 'flags' parameter is a combination of 'SubstanceMallocExFlag' enums.
	Specifies the platform memory type to allocate.<BR> 
	The 'bytesCount' parameter is the number of bytes to allocate.<BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer.

	If this callback is not defined or it returns NULL, the 
	'SubstanceCallbackMalloc' callback is used instead. If the latter is not 
	defined too, default memory allocation is used. */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackMallocEx)(
	unsigned int flags,
	unsigned int bytesCount,
	unsigned int alignment);

/** @typedef void SubstanceCallbackFreeEx
	@brief Type of the 'extended memory de-allocation' callback function

	@code void (*)(unsigned int flags,void* bufferPtr) @endcode

	This callback allows to free buffers allocated with the
	SubstanceCallbackMallocEx callback.<BR>
	The 'flags' parameter is a combination of 'SubstanceMallocExFlag' enums.
	Specifies the platform memory type to de-allocate.<BR> 
	The 'bufferPtr' parameter is the buffer to free. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackFreeEx)(
	unsigned int flags,
	void* bufferPtr);
	
/** @typedef void* SubstanceCallbackOutputMalloc
	@brief Type of the 'result output texture allocation' callback function
	@warning Only valid with the BLEND Substance platform.

	@code void* (*)(
		struct SubstanceHandle_ *handle,
		size_t jobUserData,
		unsigned int outputIndex,
		unsigned int mipmapLevel,
		unsigned int *pitch,        // Input, output
		unsigned int bytesCount,
		unsigned int alignment) @endcode

	This callback is called for each mipmap level every time an output texture 
	destination buffer is required.<BR>
	It allows the user to put the output in a specific memory location.
	
	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	'jobUserData' is the user information of the job corresponding to this
	output.<BR>
	The 'outputIndex' parameter is the index of the completed output.<BR>
	The 'mipmapLevel' parameter is the mipmap level of the output 
	(0: base level).<BR>
	The 'pitch' parameter is a pointer to the size of a pixel row (or size of a 
	block row for compressed formats) in bytes. The user must modify it if a 
	non-default pitch is required. <BR>
	The 'bytesCount' parameter is the number of bytes to allocate. It is a 
	minimal value: the allocated buffer can actually be larger if the pitch is 
	tweaked by the user, in which case this value can be discarded. <BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer. This buffer will be accessed only for 
	write operations (no read operations: the buffer returned can be in mapped 
	video memory or equivalent). */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackOutputMalloc)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int outputIndex,
	unsigned int mipmapLevel,
	unsigned int *pitch,
	unsigned int bytesCount,
	unsigned int alignment);
	
#ifdef __cplusplus
};
/* End of the EXTERNC block (if necessary) */
#endif /* ifdef __cplusplus */

#endif /* ifndef _SUBSTANCE_CALLBACKS_H */
