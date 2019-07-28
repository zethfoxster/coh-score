/** @file context.h
	@brief Substance engine context structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceContext structure. Initialized from platform specific
	informations. This structure is necessary to create Substance handles.
	The context can be used to fill in function callbacks. 
	
	Summary of functions defined in this file:
	
	Context creation and deletion:
	  - substanceContextInit
	  - substanceContextRelease

	Callback setting:
	  - substanceContextSetCallback
	  
	States management:
	  - substanceContextRestoreStates (advanced usage)
*/

#ifndef _SUBSTANCE_CONTEXT_H
#define _SUBSTANCE_CONTEXT_H



/** Platform specific API device structure */
#include "device.h"

/** Callback signature definitions */
#include "callbacks.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list */
#include "errors.h"



/** @brief Substance engine context structure pre-declaration.

	Must be initialized using the substanceContextInit function and released using
	the substanceContextRelease function. */
struct SubstanceContext_;

/** @brief Substance engine handle structure pre-declaration (C/C++ compatible). */
typedef struct SubstanceContext_ SubstanceContext;



/** @brief Substance engine context initialization from platform specific data
	@param[out] substanceContext Pointer to the Substance context to initialize.
	@param platformSpecificDevice Platform specific structure used to initialize
	the context. (cf. device.h)
	@pre The context must be previously un-initialized or released.
	@pre The platformSpecificDevice structure must be correctly filled
	@post The context is now valid (if no error occurred).
	@return Returns 0 if succeeded, an error code otherwise.

	This can use API/hardware related resources: in particular, this function
	initialize the default GPU API states required by the Substance Engine. For
	performances purpose, not all GPU states are initialized: please read the
	documentation specific to your platform implementation. If GPU API states 
	are modified by the user between the context initialization and a following
	computation, the substanceContextRestoreStates function must be called 
	(e.g. callbacks functions, time-slicing computation, tweaks change, etc.).<BR>
	Only one context can be created at a time on a given device. */
SUBSTANCE_EXPORT unsigned int substanceContextInit(
	SubstanceContext** substanceContext,
	SubstanceDevice* platformSpecificDevice);

/** @brief Substance engine context release
	@param substanceContext Pointer to the Substance context to release.
	@pre The context must be previously initialized and every handle that uses
	this context must be previously released.
	@post This context is not valid any more (it can be re-initialized).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceContextRelease(
	SubstanceContext* substanceContext);



/** @brief Set Substance engine context callback
	@param substanceContext Pointer to the Substance context to set the callback for.
	@param callbackType Type of the callback to set. See the SubstanceCallbackEnum
	enumeration in callback.h.
	@param callbackPtr Pointer to the callback function to set.
	@pre The context must be previously initialized.
	@pre The callback function signature and callback type must match
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceContextSetCallback(
	SubstanceContext* substanceContext,
	SubstanceCallbackEnum callbackType,
	void *callbackPtr);
	
/** @brief Restore initial platform dependant states
	@param substanceContext Pointer to the Substance context
	@pre The context must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise. 
	
	If GPU API states are modified by the user after the context initialization
	(e.g. the game scene is rendered), this function must be called just before
	the following computation (e.g. before substanceHandleStart call or at the 
	end of a callback function).<BR>
	Partially restore the GPU API state as when the context was created. Not all
	GPU states are initialized: please read the documentation specific to your 
	platform implementation. */
SUBSTANCE_EXPORT unsigned int substanceContextRestoreStates(
	SubstanceContext* substanceContext);

#endif /* ifndef _SUBSTANCE_CONTEXT_H */
