/** @file hardresources.h
	@brief Substance hardware resource dispatching requirements and hints.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceHardResources structure. Used by substanceHandleStart
	function for hardware resource dispatching requirements and hints
	definition.
*/

#ifndef _SUBSTANCE_HARDRESOURCES_H
#define _SUBSTANCE_HARDRESOURCES_H

/** Basic type defines */
#include <stddef.h>

/** @brief Maximum number of GPU present on the platform */
#define SUBSTANCE_GPU_COUNT_MAX 4

/** @brief Maximum number of CPU present on the platform */
#define SUBSTANCE_CPU_COUNT_MAX 32

/** @brief Maximum number of Streaming PU present on the platform */
#define SUBSTANCE_SPU_COUNT_MAX 32


/** @brief Substance hardware resource use switch

	Used by SubstanceHardResources structure. 8bits, mutually exclusive. */
typedef enum
{
	Substance_Resource_Default,          /**< Default value on this platform */
	Substance_Resource_DoNotUse,         /**< The resource must not be used */
	Substance_Resource_Partial1,         /**< Resource partial use: 1/8*/
	Substance_Resource_Partial2,         /**< Partial use 2/8 */
	Substance_Resource_Partial3,         /**< Partial use 3/8 */
	Substance_Resource_Partial4,         /**< Partial use 4/8 */
	Substance_Resource_Partial5,         /**< Partial use 5/8 */
	Substance_Resource_Partial6,         /**< Partial use 6/8 */
	Substance_Resource_Partial7,		  /**< Resource partial use: 7/8*/
	Substance_Resource_FullUse,          /**< The resource can be used 100% */

} SubstanceHardSwitch;


/** @brief Substance hardware resource dispatching requirements structure def. */
typedef struct
{
	/** @brief GPUs use hints. SubstanceHardSwitch enum value.
	
		Dispatch Substance computation load on GPUs */
	unsigned char gpusUse[SUBSTANCE_GPU_COUNT_MAX];
	
	/** @brief CPUs use hints. SubstanceHardSwitch enum value.
	
		Dispatch Substance computation load on CPUs  */
	unsigned char cpusUse[SUBSTANCE_CPU_COUNT_MAX];
	
	/** @brief SPUs use hints. SubstanceHardSwitch enum value.
	
		Dispatch Substance computation load on SPUs (Streaming processors units) */
	unsigned char spusUse[SUBSTANCE_SPU_COUNT_MAX];
	
	/** @brief System memory budget
		@note Set the maximum amount of system memory that Substance can use.
		0: no limitations. */
	size_t systemMemoryBudget;
	
	/** @brief System video budget per GPU
		@note Set the maximum amount of system memory that Substance can use.
		0: no limitations. */
	size_t videoMemoryBudget[SUBSTANCE_GPU_COUNT_MAX];
	
} SubstanceHardResources;


#endif /* ifndef _SUBSTANCE_HARDRESOURCES_H */
