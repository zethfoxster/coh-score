/** @file errors.h
	@brief Substance error codes definitions. Returned by all Substance functions.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved. */

#ifndef _SUBSTANCE_ERRORS_H
#define _SUBSTANCE_ERRORS_H


/** @brief Substance error codes enumeration */
typedef enum
{
	Substance_Error_None                  = 0, /**< No errors */
	Substance_Error_IndexOutOfBounds,          /**< In/output index overflow */
	Substance_Error_InvalidData,               /**< Substance data not recognized */
	Substance_Error_PlatformNotSupported,      /**< Substance data platform invalid */
	Substance_Error_VersionNotSupported,       /**< Substance data version invalid */
	Substance_Error_BadEndian,                 /**< Substance data bad endianness */
	Substance_Error_BadDataAlignment,          /**< Substance data not correctly aligned */
	Substance_Error_InvalidContext,            /**< Context is not valid */
	Substance_Error_NotEnoughMemory,           /**< Not enough system memory */
	Substance_Error_InvalidHandle,             /**< Invalid handle used */
	Substance_Error_ProcessStillRunning,       /**< Renderer unexpectedly running*/
	Substance_Error_NoProcessRunning,          /**< Rend. expected to be running*/
	Substance_Error_TypeMismatch,              /**< Types do not match */
	Substance_Error_InvalidDevice,             /**< Platform spec. device invalid*/
	Substance_Error_NYI,                       /**< Not yet implemented */
	Substance_Error_CannotCreateMore,          /**< Cannot create more items */
	Substance_Error_InvalidParams,             /**< Invalid parameters */
	Substance_Error_InternalError,             /**< Internal error */
	Substance_Error_NotComputed,               /**< Output(s) not already comp. */
	Substance_Error_PlatformDeviceInitFailed,  /**< Cannot init the ptfrm device */
	Substance_Error_EmptyRenderList,           /**< The render list is empty */
	Substance_Error_BadThreadUsed,             /**< API function thread misuse */
	Substance_Error_EmptyOutputsList,          /**< The Outputs list is empty */
	Substance_Error_OutputIdNotFound,          /**< Cannot found the Output Id */
	
	/* etc. */
	
	Substance_Error_Internal_Count
	
} SubstanceErrorCodes;

#endif /* ifndef _SUBSTANCE_ERRORS_H */
