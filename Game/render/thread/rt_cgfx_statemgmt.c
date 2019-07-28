/****************************************************************************
	rt_cgfx_statemgmt.c

	State management support for CGFX.
	
	Author: Aki Morita 02/2009
****************************************************************************/

#include "ogl.h"
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "SuperAssert.h"
#include "wcw_statemgmt.h"
#include "rt_cgfx.h"

static CGprofile vertexProfile;
static CGprofile fragmentProfile;

//==========================================================================
void rt_cgfx_statemgmt_init( CGcontext context )
{
#if 0 /// @todo Does not work with the current Cg 2.2 beta
	int i;

	// Set all optimal options
	for(i = 0; i < cgGetNumSupportedProfiles(); i++)
	{
		cgGLSetOptimalOptions(cgGetSupportedProfile(i));
	}
#endif

	cgGLRegisterStates(context);

	vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

	if(fragmentProfile == CG_PROFILE_UNKNOWN && rdr_caps.chip&ARBFP ) {
		printf("Removing ARBFP since cg doesn't support arbfp1 profile\n");
		rdr_caps.chip &= ~ARBFP;
	}
	if(vertexProfile == CG_PROFILE_UNKNOWN && rdr_caps.chip&(ARBVP|ARBFP) ) {
		printf("Removing ARBVP since cg doesn't support arbvp1 profile on this card\n");
		rdr_caps.chip &= ~(ARBVP|ARBFP); // no arbfp without arbvp
		rdr_caps.chip = TEX_ENV_COMBINE; // NV1X path not working?  Fall back to TEX_ENV_COMBINE
	}
}

