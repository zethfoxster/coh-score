/** @file version.h
	@brief Substance current version description structure and accessor function
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080610
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceVersion structure. Filled using the
	substanceGetCurrentVersion function.
*/

#ifndef _SUBSTANCE_VERSION_H
#define _SUBSTANCE_VERSION_H


/** Platform dependent definitions */
#include "platformdep.h"


/** @brief Substance platforms enumeration */
typedef enum
{
	Substance_Platform_Unknown   = 0x0,
	Substance_Platform_ogl2      = 0x1,
	Substance_Platform_oglcg     = 0x2,
	Substance_Platform_d3d9pc    = 0x3,
	Substance_Platform_orca      = 0x4,
	Substance_Platform_xbox360   = 0x5,
	Substance_Platform_ps3psgl   = 0x6,
	Substance_Platform_d3d10pc   = 0x7,
	Substance_Platform_ps3gcm    = 0x8,
	
	Substance_Platform_FORCEDWORD = 0xFFFFFFFF   /**< Force DWORD enumeration */
	
} SubstancePlatformsEnum;


/** @brief Substance current version description

	Filled using the substanceGetCurrentVersion function declared just below. */
typedef struct
{
	/** @brief Current engine implementation platform (enum) */
	SubstancePlatformsEnum platformImplEnum ;
	
	/** @brief Current engine implementation platform name */
	const char* platformImplName ;
	
	/** @brief Current engine version: major */
	unsigned int versionMajor;
	
	/** @brief Current engine version: minor */
	unsigned int versionMinor;
	
	/** @brief Current engine version: patch */
	unsigned int versionPatch;
	
	/** @brief Current engine stage (Release, alpha, beta, RC, etc.) */
	const char* currentStage;
	
	/** @brief Substance engine build date (Mmm dd yyyy) */
	const char* buildDate;
	
	/** @brief Substance engine sources revision */
	unsigned int sourcesRevision;

} SubstanceVersion;


/** @brief Get the current substance engine version
	@param[out] version Pointer to the version struct to fill. */
SUBSTANCE_EXPORT void substanceGetCurrentVersion(SubstanceVersion* version);


#endif /* ifndef _SUBSTANCE_VERSION_H */
