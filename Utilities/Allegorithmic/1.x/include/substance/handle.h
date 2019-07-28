/** @file handle.h
	@brief Substance engine handle structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.
	@version 2137
	  - substanceHandlePushOutputs function modified: output selection from
	    texture sets removed.
	  - substanceHandleGetOutputs function modified: SubstanceGrabMethod
	    enumeration removed (only Grab method is available).<BR>
		Replaced by output interpretation flag.

	Defines the SubstanceHandle structure. Initialized from a SubstanceContext
	and a buffer that contains the Substance cooked data to render.
	This handle is used to set up generation parameters, select textures to
	render, fill-in inputs, launch the generation process and grab resulting
	textures.<BR>
	The rendering process can be launched synchronously or asynchronously (in
	one go or using time-slicing). The process workload can be dispatched on
	the different hardware units.<BR>
	Generated textures can be grabbed progressively using a callback
	function or at a stretch at the end of generation.

	Summary of functions defined in this file:

	Handle creation and deletion:
	  - substanceHandleInit
	  - substanceHandleRelease

	Render list management:
	  - substanceHandlePushOutputs
	  - substanceHandlePushSetInput (advanced usage)
	  - substanceHandleFlush (advanced usage)

	Generation process start/stop, resources switch, and state accessor:
	  - substanceHandleStart
	  - substanceHandleStop (advanced usage)
	  - substanceHandleSwitchHard (advanced usage)
	  - substanceHandleGetState (advanced usage)

	Grab Results:
	  - substanceHandleGetOutputs

	Substance data description accessors:
	  - substanceHandleGetDesc
	  - substanceHandleGetOutputDesc
	  - substanceHandleGetInputDesc (advanced usage)
	  
	Synchronisation:
	  - substanceHandleSyncSetLabel (advanced usage)
	  - substanceHandleSyncGetLabel (advanced usage)

	Typical call sequence for Substance handle (synchronous, grab all texture):
	  - 1. substanceHandleInit (initialization from Substance cooked data)
	  - 2. substanceHandleGetDesc (retrieve the number of outputs)
	  - 3. substanceHandlePushOutputs (request the needed output textures)
	  - 4. substanceHandleStart (launch the rendering process)
	  - 5. substanceHandleGetOutputs (grab the outputs textures)
	  - 6. substanceHandleRelease (close the handle)
*/

#ifndef _SUBSTANCE_HANDLE_H
#define _SUBSTANCE_HANDLE_H


/** Context structure and related functions definitions */
#include "context.h"

/** Texture (results) structure and related functions definitions */
#include "texture.h"

/** Callbacks signatures definitions */
#include "callbacks.h"

/** Substance data description definitions */
#include "datadesc.h"

/** Output description definitions */
#include "outputdesc.h"

/** Input description definitions */
#include "inputdesc.h"

/** Texture used as input, contains platform specific texture and format info */
#include "textureinput.h"

/** Hardware resource dispatching requirements definitions */
#include "hardresources.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list */
#include "errors.h"

/** Basic type defines */
#include <stddef.h>


/** @brief Render list push options (flags)

	Used by the substanceHandlePush[...] functions.

	Allows to set the 'Hint only' flag */
typedef enum
{
	/* Hint only flag (the action to push is only a hint that helps Substance to
		anticipate which texture is likely to be required soon). */
	Substance_PushOpt_NotAHint       = 0x0,     /**< Not a hint (default) */
	Substance_PushOpt_HintOnly       = 0x800,   /**< Hint only flag */

} SubstancePushOption;


/** @brief Selection list interpretation flag

	Used by the substanceHandlePushOutputs
		and substanceHandleGetOutputs functions.

	Allows to select how the values in the selection list should be interpreted:
	  - textures are selected by output indices
	  - textures are selected by identifiers */
typedef enum
{
	/* Selection list interpretation (mutually exclusive flags) */
	Substance_OutOpt_OutIndex   = 0x0, /**< Index output selection (default) */
	Substance_OutOpt_TextureId  = 0x2, /**< Texture unique identifiers output selection */

	/* Selection list allocation flags. */
	Substance_OutOpt_CopyNeeded = 0x0000, /**< Volatile Selec. list, copy needed (default)*/
	Substance_OutOpt_DoNotCopy  = 0x1000, /**< Persistent Selection list */

} SubstanceOutputSelectionOption;


/** @brief Synchronization options. */
typedef enum
{
	/* Synchronization mode (mutually exclusive flags) */
	Substance_Sync_Synchronous  = 0x0,  /**< Synchronous computation */
	Substance_Sync_Asynchronous = 0x1,  /**< Asynchronous computation */

} SubstanceSyncOption;


/** @brief Start options.

	Used by substanceHandleStart 'flags' parameter. */
typedef enum
{
	/* Ending option (mutually exclusive flags) */
	Substance_Ending_Stop    = 0x0, /**< Process stopped on end */
	Substance_Ending_Waiting = 0x2, /**< Process waiting, explicit stop needed*/

} SubstanceStartOption;


/** @brief Substante state flags.

	Used by substanceHandleGetState 'state' output parameter. */
typedef enum
{
	Substance_State_Started        = 0x1,  /**< Process currently running (Started) */
	Substance_State_Asynchronous   = 0x2,  /**< Asynchronous process running */
	Substance_State_EndingWaiting  = 0x4,  /**< Process started with Substance_Ending_Waiting option */
	Substance_State_NoMoreRender   = 0x8,  /**< The render list is empty */
	Substance_State_NoMoreRsc      = 0x10, /**< Process idle: no hardware resources available */
	Substance_State_PendingCommand = 0x20, /**< User command (API calls) pending */

} SubstanceStateFlag;



/** @brief Substance engine handle structure pre-declaration.

	Must be initialized using the substanceHandleInit function and released
	using the substanceHandleRelease function. */
struct SubstanceHandle_;

/** @brief Substance engine handle structure pre-declaration (C/C++ compatible).
*/
typedef struct SubstanceHandle_ SubstanceHandle;



/** @brief Substance engine handle initialization from context and data
	@param[out] substanceHandle Pointer to the Substance handle to initialize.
	@param substanceContext Pointer to the Substance context used for the
		rendering of substance data.
	@param substanceDataPtr Pointer to the substance cooked data to render.
	@param substanceDataSize Size of the substanceDataPtr buffer in bytes.
		Used only for debug purposes. Can be 0 (no overflow checks can be done).
	@param[in] substanceRscInitial Pointer to initial hardware resource
		dispatching. Set this parameter to NULL to use the default parameters
		of the engine for the platform.
	@pre The handle must be previously un-initialized or released.
	@pre The context must be valid (correctly initialized) and remain valid
	during the handle lifetime.
	@pre The Substance data must be present and valid during the handle
		lifetime. It must be at least 16 bytes aligned.
	@post The handle is now valid (if no errors occurred).
	@return Returns 0 if succeeded, an error code otherwise.

	Dynamic allocations of intermediate scratch buffers are done in this
	function.
	This function is quite fast (## to evaluate ##) and does not use
	API/hardware related resources (i.e. it is GPU-calls-free). */
SUBSTANCE_EXPORT unsigned int substanceHandleInit(
	SubstanceHandle** substanceHandle,
	SubstanceContext* substanceContext,
	unsigned char* substanceDataPtr,
	size_t substanceDataSize,
	const SubstanceHardResources* substanceRscInitial);

/** @brief Substance engine handle release
	@param substanceHandle Pointer to the Substance handle to release.
	@pre The handle must be already initialized and there must be no generation
		processes currently running.
	@post This handle is not valid any more (it can be re-initialized).
		Non-grabbed result textures are released.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleRelease(
	SubstanceHandle* substanceHandle);




/** @brief Push a new set of outputs in the render list.
	@param substanceHandle Pointer to the Substance handle.
	@param flags Generation flags. A combination of SubstancePushOption and
		SubstanceOutputSelectionOption flags. These flags allow to choose the
		selectList parameter behavior and to set the hint flag.
	@param[in] selectList Pointer to the array of textures to render. Can be
		NULL: all data outputs are selected. Otherwise the array is interpreted
		according to the 'flags' parameter value:
		<UL>
			<LI>Substance_OutOpt_OutIndex (default): the list of integers is
			interpreted as output indices.</LI>
			<LI>Substance_OutOpt_TextureId: list of output unique
			identifiers.</LI>
		</UL>
		By default, the selectList data is copied by Substance. If the data is
		persistent (always present in memory), set the
		Substance_OutOpt_DoNotCopy flag to avoid copy.
	@param selectListCount Number of entries in the selectList array.
	@param jobUserData User data (integer/pointer) to associate with the job.
		This value is transmited to callback functions: it allows tracking of
		the job lifetime. This parameter is optional, e.g. set to 0 by default.
	@pre The handle must be previously initialized.
	@pre If the Substance_OutOpt_DoNotCopy flag is set, the selectList buffer
		must remain valid during the job lifetime.
	@return Returns 0 if succeeded, an error code otherwise.

	A call to this function describes a job. This job is added at the end of the
	render list. Pending jobs are not affected.

	If the Substance_PushOpt_HintOnly flag is set, the outputs are pushed into
	the render list as usual, but they will not be rendered. This feature allows
	the game engine to send hints to Substance (based on the game level design
	and/or the player behavior). These hints allow Substance to anticipate
	future computations and manage internal pools accordingly.<BR>
	Hints must be pushed at the end of the render list: Non-hints jobs cannot be
	appended after hints jobs.

	The outputs of the list are not rendered in the order of the list. However
	the jobs of the render list are processed sequentially without any
	reordering.*/
SUBSTANCE_EXPORT unsigned int substanceHandlePushOutputs(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	const unsigned int selectList[],
	unsigned int selectListCount,
	size_t jobUserData);

/** @brief Set input (render param.) value. Action appended in the render list.
	@param substanceHandle Pointer to the Substance handle.
	@param flags Combination of SubstancePushOption flags. These flags allow to
		choose the way this item is added to the render list and the hint flag.
	@param inIndex Index of the input to set the value for.
	@param inType Type of the value
	@param value Pointer to value. Can be NULL if Substance_PushOpt_HintOnly
		flag is set.<BR>
		The concrete type of the value depends on inType:
		  - Substance_IType_Float, Substance_IType_Float2/3/4:
		    array of 1/2/3/4 float(s).
		  - Substance_IType_Integer: an int value.
		  - Substance_IType_Image: a pointer to a SubstanceTextureInput
		    structure. Must be correctly filled and remain valid during the
			handle lifetime.
		  - Substance_IType_Mesh: ## to determine, NYI ##.
		  - Substance_IType_String: an ASCII or UNICODE string
		    (platform dependent).
	@param jobUserData User data (integer/pointer) to associate with the render
		list item. This value is transmited to callback functions: it allows
		tracking of the render list item lifetime. This parameter is optional,
		e.g. set to 0 by default.
	@pre The handle must be previously initialized and valid.
	@pre The input index must be valid (< inputCount).
	@pre The concrete type of the input and inType must match. Except if the
		Substance_PushOpt_HintOnly flag is set.
	@return Returns 0 if succeeded, an error code otherwise.

	A call to this function does not modify the input immediatly, but pushes a
	new item in the render list. Like substanceHandlePushOutputs, this function
	handles the Substance_PushOpt_HintOnly flag (see substanceHandlePushOutputs
	description for further information). */
SUBSTANCE_EXPORT unsigned int substanceHandlePushSetInput(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	unsigned int inIndex,
	SubstanceInputType inType,
	void* value,
	size_t jobUserData);
	
/** @brief Flush the render list.
	@param substanceHandle Pointer to the Substance handle.
	@return Returns 0 if succeeded, an error code otherwise.

	Flush ALL the items previously pushed in the render list. The process is not
	stopped: if other jobs are pushed just after the flush commands, the process
	proceed to these new jobs.

	The internal cache is NOT flushed by this function. */
SUBSTANCE_EXPORT unsigned int substanceHandleFlush(
	SubstanceHandle* substanceHandle);




/** @brief Computation process launch, process the render list
	@param substanceHandle Pointer to the Substance handle to compute.
	@param flags Combination of SubstanceStartOption and SubstanceSyncOption
		enumerations.<BR>
		Set Substance_Sync_Asynchronous to launch the process in another thread
		and to return immediatly (does not wait for process termination).<BR>
		Set Substance_Ending_Waiting to force asynchronous process waiting for
		further computation at render list ending. When this flag is set, an
		explicit stop (substanceHandleStop) is needed. Ignored if
		Substance_Sync_Asynchronous flag is not set.
	@pre The handle must be previously initialized, the render list must be
		previously filled by substanceHandlePushOutputs calls.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		Returns immediatly if asynchronous mode is selected.<BR>
		Returns at the end of computation otherwise, i.e.:
		  - The end of the render list is reached
		  - Through substanceHandleStop call (from another thread or through
		  a callback function).
		  - Current/switched hardware resource dispatching does not allocate any
		  process unit (see substanceHandleSwitchHard function).

	This function is used to consume the render list. */
SUBSTANCE_EXPORT unsigned int substanceHandleStart(
	SubstanceHandle* substanceHandle,
	unsigned int flags);

/** @brief Cancel/Stop the computation process.
	@param substanceHandle Pointer to the Substance handle.
	@param syncMode Set Substance_Sync_Asynchronous to return immediatly (does
		not wait for the actual process termination).
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		Returns immediatly if syncMode is Substance_Sync_Asynchronous or the
		current process is launched synchronously (allows call through a
		callback function or from another thread).<BR>
		Returns at actual computation end otherwise.

	Stops the computation process. DOES NOT flush the render list.<BR>
	This function can be called at any time for processes started both in
	asynchronous or synchronous mode. */
SUBSTANCE_EXPORT unsigned int substanceHandleStop(
	SubstanceHandle* substanceHandle,
	SubstanceSyncOption syncMode);

/** @brief On the fly switch to another hardware resource dispatching
	@param substanceHandle Pointer to the Substance handle.
	@param syncMode Synchronous flag, handled differently depending on whether
		the duration is finite or not, a current process is running or not and
		this process is synchronous or not (see the returned value description).
	@param[in] substanceRscNew Pointer to new hardware resource dispatching
		requirements and hints. Set this parameter to NULL in order to use
		the initial resource dispatching given at the substanceHandleInit
		function call (or platform default ones if not given).
	@param[in] substanceRscNext Pointer to hardware resource dispatching used
		at window time end (see the 'duration' parameter). Set this parameter
		to NULL in order to use initial resource dispatching
		(see substanceRscNew). Meaningful only if duration is finite.
	@param duration Duration of the switch in nanoseconds. Can be 0: INFINITE.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		<TABLE><CAPTION>Return behavior</CAPTION>
		<TR>
			<TD><I>Parameters --></I></TD>
			<TD><B>Finite/Synchronous</B></TD>
			<TD><B>Finite/Asynchronous</B></TD>
			<TD><B>Infinite/Synchronous</B></TD>
			<TD><B>Infinite/Asynchronous</B></TD>
		</TR>
		<TR>
			<TD><B>No process running</B></TD>
			<TD>Returns immediatly</TD>
			<TD>Returns immediatly</TD>
			<TD>Returns immediatly</TD>
			<TD>Returns immediatly</TD>
		</TR>
		<TR>
			<TD><B>Asynchronous process</B></TD>
			<TD>Returns at duration end</TD>
			<TD>Returns immediatly</TD>
			<TD>Returns when the switch is effective</TD>
			<TD>Returns immediatly</TD>
		</TR>
		<TR>
			<TD><B>Synchronous process</B></TD>
			<TD>Returns immediatly (1)</TD>
			<TD>Returns immediatly (1)</TD>
			<TD>Returns immediatly (1)</TD>
			<TD>Returns immediatly (1)</TD>
		</TR></TABLE>
		(1): Allows call through a callback function or from another thread.

	This function allows to change the hardware resource dispatching during
	or before a computation (e.g. use a GPU between vsync idle time:
	time-slicing strategy).<BR>
	The change is not limited in time if 'duration' == 0 (INFINITE).
	Otherwise, the switch is clocked: 'duration' defines a time range in
	nanoseconds.

	The process can be put in standby if the SubstanceHardResources does not
	allocate any process unit.  */
SUBSTANCE_EXPORT unsigned int substanceHandleSwitchHard(
	SubstanceHandle* substanceHandle,
	SubstanceSyncOption syncMode,
	const SubstanceHardResources* substanceRscNew,
	const SubstanceHardResources* substanceRscNext,
	unsigned int duration);

/** @brief Get the computation process state
	@param substanceHandle Pointer to the Substance handle to get the state
		from.
	@param[out] jobUserData If not NULL, returns the user data associated with
		the render list item currently processed. Returns 0 if no process is
		currently running.
	@param[out] state If not NULL, returns an integer containing the current
		state: combination of SubstanceStateFlag flags.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.

	This function can be called at any time for processes started both in
	asynchronous or synchronous mode. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetState(
	const SubstanceHandle* substanceHandle,
	size_t *jobUserData,
	unsigned int* state);


/** @brief Grab output texture results
	@param substanceHandle Pointer to the Substance handle to grab texture from.
	@param flags One of SubstanceOutputSelectionOption flags (only
		Substance_OutOpt_TextureIndex or Substance_OutOpt_TextureId flags are
		supported). Set to 0 to simply grab textures from its index numbers.
	@param outIndexOrId Index of the first output to grab or Id of the texture
		to grab if the Substance_OutOpt_TextureId flag is set.
	@param outCount Number of outputs to grab. Only significant if
		Substance_OutOpt_TextureId flag is not set (otherwise only 1 texture
		can be grab at a time).
	@param[out] substanceTexture Pointer to the array of texture structures to
		fill. Its size must be equal or greater than outCount or >= to 1 if the
		Substance_OutOpt_TextureId flag is set.
	@pre The handle must be previously initialized and valid.
	@pre The output index and count must be valid
		(outIndex + outCount =< outputCount).
	@post The corresponding outputs are transferred to the caller. The ownership
		is also transferred: the user have to free/release the texture when
		necessary.
	@note Is it not necessary to use this function for the output textures
		allocated via the SubstanceCallbackOutputMalloc callback: these textures
		are NOT stored by Substance and NULL pointers will be returned by this
		function.
	@warning Result textures are NOT counted in the memory budget of the
		Substance handle. Memory budget must be adjusted if necessary.
	@return Returns 0 if succeeded, an error code otherwise (returns
		Substance_Error_NotComputed if the output is not already computed).

	This function can be called at any time, even during a computation. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetOutputs(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	unsigned int outIndexOrId,
	unsigned int outCount,
	SubstanceTexture substanceTexture[]);




/** @brief Get the handle main description
	@param[in] substanceHandle Pointer to the Substance handle of which to get
		the description.
	@param[out] substanceInfo Pointer to the information structure to fill.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetDesc(
	const SubstanceHandle* substanceHandle,
	SubstanceDataDesc* substanceInfo);

/** @brief Get an output (texture result) description and current state
	@param[in] substanceHandle Pointer to the Substance handle.
	@param outIndex Index of the output of which to get the description.
	@param[out] substanceOutDesc Pointer to the texture information struct to
		fill.
	@pre The handle must be previously initialized and valid.
	@pre The output index must be valid (< outputCount).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetOutputDesc(
	const SubstanceHandle* substanceHandle,
	unsigned int outIndex,
	SubstanceOutputDesc* substanceOutDesc);

/** @brief Get an input (render parameter) description
	@param[in] substanceHandle Pointer to the Substance handle.
	@param inIndex Index of the input of which to get the description.
	@param[out] substanceInDesc Pointer to the input information struct to fill.
	@pre The handle must be previously initialized and valid.
	@pre The input index must be valid (< inputCount).
	@return Return 0 if succeed, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetInputDesc(
	const SubstanceHandle* substanceHandle,
	unsigned int inIndex,
	SubstanceInputDesc* substanceInDesc);

	
/** @brief Synchronization function: set the handle synchronization label value
    @param substanceHandle Pointer to the Substance handle.
    @param labelValue The value to assign to the handle synchronization label.
		This value will be really set when this call is processed by the
		Substance internal command buffer loop. 
    @return Returns 0 if succeeded, an error code otherwise. 
	
	The synchronization label value can be accessed with the 
	substanceHandleSyncGetLabel function.<BR>
	This function allows the user to check if previous commands were already 
	processed by the Substance handle. It can be used for discarding deprecated 
	callbacks. */
SUBSTANCE_EXPORT unsigned int substanceHandleSyncSetLabel(
    SubstanceHandle* substanceHandle,
    size_t labelValue);

/** @brief Synchronization function: accessor to the synchronization label value
    @param substanceHandle Pointer to the Substance handle.
    @param[out] labelValuePtr Pointer to the variable that will receive the 
		label value. 
    @return Returns 0 if succeeded, an error code otherwise. 
	
	The synchronization label value is modified with the 
	substanceHandleSyncSetLabel function.<BR>
	The default label value is 0. */
SUBSTANCE_EXPORT unsigned int substanceHandleSyncGetLabel(
    SubstanceHandle* substanceHandle,
    size_t* labelValuePtr);


#endif /* ifndef _SUBSTANCE_HANDLE_H */
