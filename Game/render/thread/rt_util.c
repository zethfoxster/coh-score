#define RT_PRIVATE
#include "error.h"
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "file.h"
#include "nvparse.h"
#include "rt_tune.h"
#include "cmdgame.h"
#include "gfxDebug.h"
#include "rt_util.h"

static GLuint *stencilbuf;
static GLubyte *colorbuf;

void rdrInitDepthComplexity()
{
	glClear(GL_STENCIL_BUFFER_BIT); CHECKGL;

	//GLint vp[4];
	if (!stencilbuf)
	{
		stencilbuf = malloc(1600*1200*sizeof(GLuint));
		colorbuf = malloc(3*1600*1200*sizeof(GLubyte));
	}

    WCW_EnableStencilTest();
    WCW_StencilFunc( GL_ALWAYS, 1, ~0 );
    WCW_StencilOp( GL_INCR, GL_INCR, GL_INCR );

	//glGetIntegerv( GL_VIEWPORT, vp ); CHECKGL;
    //glReadPixels(0,0,vp[2],vp[3],GL_STENCIL_INDEX,GL_UNSIGNED_INT,stencilbuf); CHECKGL;
}

void rdrDoDepthComplexity(F32 fps)
{
    static GLint vp[4];
    int i;
    float total;

    WCW_DisableStencilTest();
    glGetIntegerv( GL_VIEWPORT, vp ); CHECKGL;
    glReadPixels(0,0,vp[2],vp[3],GL_STENCIL_INDEX,GL_UNSIGNED_INT,stencilbuf); CHECKGL;
    total = 0; 
    for ( i = 0; i < vp[2]*vp[3]; i++ ) 
        {
        //colorbuf[3*i+0] = 2*stencilbuf[i];
        //colorbuf[3*i+1] = 2*stencilbuf[i];
        //colorbuf[3*i+2] = 2*stencilbuf[i];
        total += stencilbuf[i];
        }

		/*
    glClear( GL_DEPTH_BITS ); CHECKGL;
    glMatrixMode( GL_PROJECTION ); CHECKGL;
    glPushMatrix(); CHECKGL;
    glLoadIdentity(); CHECKGL;
    gluOrtho2D( 0, vp[2], 0, vp[3] ); CHECKGL;
    glMatrixMode( GL_MODELVIEW ); CHECKGL;
    glPushMatrix(); CHECKGL;
    glLoadIdentity(); CHECKGL;
    glRasterPos2i(0,0); CHECKGL;
    WCW_DisableBlend();
	WCW_Fog(0);
    glDrawPixels( vp[2], vp[3], GL_RGB, GL_UNSIGNED_BYTE, colorbuf ); CHECKGL;
    //WCW_EnableBlend();
    glPopMatrix(); CHECKGL;
    glMatrixMode( GL_PROJECTION ); CHECKGL;
    glPopMatrix(); CHECKGL;
	*/
#if RDRMAYBEFIX
	xyprintf(1,14, "(#s change based on camera position)");  
    xyprintf(1,15, "resolution: (%dx%d)", vp[2], vp[3] ); 
    xyprintf(1,16, "particle depth complex: %.2f", total/(vp[2]*vp[3]) );
    xyprintf(1,17, "particle fillrate: %.2f Mps", (total/(1024.0*1024.0))*fps );
	WCW_EnableStencilTest();
	WCW_StencilFunc(GL_ALWAYS, 128, ~0);
	WCW_StencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
#endif
}

int renderNvparse(char *str)
{ 
	nvparse(str);
	{
	FILE	*file;
	int		len;
	char	*mem;

		file = fileOpen("c:/temp/nvparse_errs.txt","w+");
		if (file)
		{
			nvparse_print_errors(fileRealPointer(file));
			len = ftell(file);
			if (len)
			{
				mem = malloc(len+1);
				fseek(file,0,0);
				fread(mem,len,1,file);
				mem[len] = 0;
			}
			fclose(file);
			if (len)
			{
				printf("Nvparse err: %s\n",mem);
				return 0;
			}
		}
	}
	return 1;
}

// hook up the tuning/tweak menu for some of the render debug features
void rt_InitDebugTuningMenuItems(void)
{
#ifndef FINAL
	tunePushMenu("Render Debug/Info");
	{
		tuneBit("Show render info", &game_state.renderinfo, 1, NULL);
		{
			extern int g_wcw_disable_state_management;
			tuneBit("Disable State Management", &g_wcw_disable_state_management, 1, NULL);
		}
		tuneBit("Skip CheckGLState calls", &game_state.gfxdebug, GFXDEBUG_SKIP_CHECKGLSTATE, NULL);
		{
			extern int g_wcw_disable_checkgl;
			tuneBit("Disable CHECKGL calls (e.g. for API trace)", &g_wcw_disable_checkgl, 1, NULL);
		}
		tuneBit("Enable GPU Tracking Markers", &game_state.gpu_markers, 1, NULL);
		tuneBit("Force use of fallback materials", &game_state.force_fallback_material, 1, NULL);

		tuneBit("Show Normal Vectors", &game_state.gfxdebug, GFXDEBUG_NORMALS, NULL);
		tuneBit("Show Binormal Vectors", &game_state.gfxdebug, GFXDEBUG_BINORMALS, NULL);
		tuneBit("Show Tangent Vectors", &game_state.gfxdebug, GFXDEBUG_TANGENTS, NULL);
		tuneFloat("Normal Vector Display Scale", &game_state.normal_scale, 0.01f, 0.0f, 1.0f, NULL);
		tuneBit("Don't Show Left Handed TBN", &game_state.gfxdebug, GFXDEBUG_TBN_SKIP_LEFT_HANDED, NULL);
		tuneBit("Don't Show Right Handed TBN", &game_state.gfxdebug, GFXDEBUG_TBN_SKIP_RIGHT_HANDED, NULL);
		tuneBit("Flip TBN Handedness on load", &game_state.gfxdebug, GFXDEBUG_TBN_FLIP_HANDEDNESS, NULL);
		tuneBit("Show Seams", &game_state.gfxdebug, GFXDEBUG_SEAMS, NULL);
		tuneBit("Show Light Dirs", &game_state.gfxdebug, GFXDEBUG_LIGHTDIRS, NULL);
		tuneBit("Show Vert Info", &game_state.gfxdebug, GFXDEBUG_VERTINFO, NULL);

		tuneBit("Show Alpha Sorted", &game_state.tintAlphaObjects, 1, NULL);
		tuneBit("Show Occluders", &game_state.tintOccluders, 1, NULL);		//(=green, partial=blue,not=red)

		tuneInteger("Only Draw Mesh Index", &game_state.meshIndex, 1, -1, 200, NULL);
		tuneInteger("Only Draw Mesh Sub Index", &g_client_debug_state.submeshIndex, 1, -1, 15, NULL);
		tuneInteger("Only This Sprite Index", &g_client_debug_state.this_sprite_only, 1, -1, 1000, NULL);

		tunePopMenu();

		tunePushMenu("Render Control");
		tuneBit("Skip 2D Rendering", &game_state.perf, GFXDEBUG_PERF_SKIP_2D, NULL);
		tuneBit("Skip Sky Rendering", &game_state.perf, GFXDEBUG_PERF_SKIP_SKY, NULL);
		tuneBit("Skip GroupDef graph traversal", &game_state.perf, GFXDEBUG_PERF_SKIP_GROUPDEF_TRAVERSAL, NULL);	// also used in WCW_TexLODBias
		tuneBit("Skip GfxTree graph traversal", &game_state.perf, GFXDEBUG_PERF_SKIP_GFXNODE_TREE_TRAVERSAL, NULL);
		tuneBit("Skip Opaque Pass", &game_state.perf, GFXDEBUG_PERF_SKIP_OPAQUE_PASS, NULL);
		tuneBit("Skip Alpha Pass", &game_state.perf, GFXDEBUG_PERF_SKIP_ALPHA_PASS, NULL);
		tuneBit("Skip Under Water Alpha Pass", &game_state.perf, GFXDEBUG_PERF_SKIP_UNDERWATER_ALPHA_PASS, NULL);
		tuneBit("Skip Lines Pass", &game_state.perf, GFXDEBUG_PERF_SKIP_LINES, NULL);
		tuneBit("Skip AVSN Sort by Type", &game_state.perf, GFXDEBUG_PERF_SKIP_SORT_BY_TYPE, NULL);
		tuneBit("Skip AVSN Sort by Distance", &game_state.perf, GFXDEBUG_PERF_SKIP_SORT_BY_DIST, NULL);
		tuneBit("Skip Sorting on sortModels", &game_state.perf, GFXDEBUG_PERF_SKIP_SORTING, NULL);
		tuneBit("Force AVSN Sort by Type", &game_state.perf, GFXDEBUG_PERF_FORCE_SORT_BY_TYPE, NULL);
		tuneBit("Force AVSN Sort by Distance", &game_state.perf, GFXDEBUG_PERF_FORCE_SORT_BY_DIST, NULL);
		tuneBit("Sort by reverse distance", &game_state.perf, GFXDEBUG_PERF_SORT_BY_REVERSE_DIST, NULL);
		tuneBit("Sort randomly", &game_state.perf, GFXDEBUG_PERF_SORT_RANDOMLY, NULL);
	}
	tunePopMenu();

	tunePushMenu("FX");
	{
		static int separator1;
		tuneBit("FX Debug Basic", &game_state.fxdebug, FX_DEBUG_BASIC, NULL);
		tuneBit("FX Show Details", &game_state.fxdebug, FX_SHOW_DETAILS, NULL);
		tuneBit("FX Print Verbose", &game_state.fxdebug, FX_PRINT_VERBOSE, NULL);
		tuneBit("FX Show Sound Info", &game_state.fxdebug, FX_SHOW_SOUND_INFO, NULL);
		tuneInteger("------------------", &separator1, 1, 0, 0, NULL);
		tuneBit("No Particles", &game_state.noparticles, 1, NULL);
		tuneBit("White particles", &game_state.whiteParticles, 1, NULL);
		tuneInteger("Maximum Particles", &game_state.maxParticles, 50, 0, 100000, NULL);
		tuneInteger("Only Particle Sys Index", &g_client_debug_state.particles_only_index, 1, 0, 2000, NULL);
		tuneBit("Skip Particle Sys Update", &game_state.fxdebug, FX_DISABLE_PART_CALCULATION, NULL);
		tuneBit("Skip Particle Sys Write", &game_state.fxdebug, FX_DISABLE_PART_ARRAY_WRITE, NULL);
		tuneBit("Skip Particle Sys Render", &game_state.fxdebug, FX_DISABLE_PART_RENDERING, NULL);
		tuneBit("Hide all Fx Geo", &game_state.fxdebug, FX_HIDE_FX_GEO, NULL);
	}
	tunePopMenu();
#endif
}

