/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "allegorithmic.h"
#include "file.h"
#include "cmdgame.h"
#include "utils.h"
#include "substance/substance.h"

static SubstanceContext *context;
static SubstanceHandle *handle; //
static SubstanceDevice device;

static char *allegorithmic_errors[] = {
    "success", 
    "Substance_Error_IndexOutOfBounds",          /**< In/output index overflow */
    "Substance_Error_InvalidData",               /**< Substance data not recognized */
    "Substance_Error_PlatformNotSupported",      /**< Substance data platform invalid */
    "Substance_Error_VersionNotSupported",       /**< Substance data version invalid */
    "Substance_Error_BadEndian",                 /**< Substance data bad endianness */
    "Substance_Error_BadDataAlignment",          /**< Substance data not correctly aligned */
    "Substance_Error_InvalidContext",            /**< Context is not valid */
    "Substance_Error_NotEnoughMemory",           /**< Not enough system memory */
    "Substance_Error_InvalidHandle",             /**< Invalid handle used */
    "Substance_Error_ProcessStillRunning",       /**< Renderer unexpectedly running*/
    "Substance_Error_NoProcessRunning",          /**< Rend. expected to be running*/
    "Substance_Error_TypeMismatch",              /**< Types do not match */
    "Substance_Error_InvalidDevice",             /**< Platform spec. device invalid*/
    "Substance_Error_NYI",                       /**< Not yet implemented */
    "Substance_Error_CannotCreateMore",          /**< Cannot create more items */
    "Substance_Error_InvalidParams",             /**< Invalid parameters */
    "Substance_Error_InternalError",             /**< Internal error */
    "Substance_Error_NotComputed",               /**< Output(s) not already comp. */
    "Substance_Error_PlatformDeviceInitFailed",  /**< Cannot init the ptfrm device */
    "Substance_Error_EmptyRenderList",           /**< The render list is empty */
    "Substance_Error_BadThreadUsed",             /**< API function thread misuse */
    "Substance_Error_EmptyOutputsList",          /**< The Outputs list is empty */
    "Substance_Error_OutputIdNotFound",          /**< Cannot found the Output Id */
};
STATIC_ASSERT(ARRAY_SIZE(allegorithmic_errors) == Substance_Error_Internal_Count);

char *allegorithmic_errorstr(int ec)
{
    return AINRANGE(ec,allegorithmic_errors) ? allegorithmic_errors[ec] : "invalid error index"; 
}

// #define ALLEGORITHMIC_ENABLED
#ifdef ALLEGORITHMIC_ENABLED
 
void allegorithmic_init(void)
{
    int ec;
    if(game_state.allegorithmic_disabled)
        return;

    if(0 != (ec = substanceContextInit(&context,&device)))
    {
        log_printf(0,"allegorithmic substance failed to init: %s",allegorithmic_errorstr(ec));
        game_state.allegorithmic_disabled = 1;
        return;
    }
    game_state.allegorithmic_enabled = 1;
}

SubstanceHandle* allegorithmic_handlefromfile(char *file)
{
    int n;
    char *d = fileAlloc(file,&n); 
    SubstanceHandle *res = 0;
    int ec;
    
    if(0 != (ec=substanceHandleInit(&res,context,d,n,NULL)))
    {
        ErrorFilenamef(file,"failed to load file, error %s",allegorithmic_errorstr(ec));
        free(d);
        return NULL;
    }

    free(d);
    return NULL;
}


// *modelGetFinalTexturesInline
SubstanceHandle *allegorithmic_debugtexture(void)
{
    if(!game_state.allegorithmic_enabled)
        return NULL;
    return allegorithmic_handlefromfile("texture_library/allegorithmic/debug.sbsbin");
}

#endif  // ALLEGORITHMIC_ENABLED
