#ifndef RT_SHADER_MGR_H
#define RT_SHADER_MGR_H

#include "rt_state.h"

// tables of gl/cg generated gpu program names (ids)
extern int shaderMgrVertexPrograms[];            // Index: DrawModeType enum type. DRAWMODE_SPRITE, etc.
extern int shaderMgrVertexProgramsHQ[];          // Index: DrawModeType enum type. DRAWMODE_SPRITE, etc.

typedef enum
{
	kCompileFlag_ARB		= 0x00000000,
	kCompileFlag_Cg			= 0x00000001,
	kCompileFlag_NotPosInv	= 0x00000002,	// Can't use the -posinv ("position invariant") option for Cg shaders
} tShaderCompileFlags;

void shaderMgr_SetCgShaderPath( const char* szPath );
void shaderMgr_GetShaderSrcFileTextCG(const char* cohRelativePathName, char** ppProgramText );
void shaderMgr_MakeFullShaderSrcPath( const char* cohRelativePathName, char* buffer, int buffSz );
void shaderMgr_PrepareShaderCache( void );
void shaderMgr_SetOverrideShaderDir( void );
void shaderMgr_InitFPs(void);
void shaderMgr_InitVPs(void);
void shaderMgr_ReleaseShaderIds(void);
void shaderMgr_ClearShaderCache(void);
int	shaderMgr_lookup_fp( BlendModeType blend_type );	// return the shader program id for this material blend mode and variation
void shaderMgr_ShaderDebugControl( void* data );		// control commands for shader debug/hud mode

#endif // RT_SHADER_MGR_H
