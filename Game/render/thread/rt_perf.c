#define RT_PRIVATE
#include "rt_perf.h"
#include "ogl.h"
#include "timing.h"
#include "rt_model_cache.h"
#include "rt_model.h"
#include "rt_tex.h"
#include "mathutil.h"
#include "assert.h"
#include "wcw_statemgmt.h"
#include "rt_shaderMgr.h"
#include "rt_win_init.h"
#include "gfxDebug.h"


RdrPerfIO *g_perfio;

static INLINEDBG void bindBufferRGBs(VBO *vbo, U8 *rgbs)
{
	if (modelBindBuffer(vbo))
	{
		if (vbo->sts2)
			texBindTexCoordPointer(TEXLAYER_GENERIC,vbo->sts2);

		texBindTexCoordPointer(TEXLAYER_BASE,vbo->sts);
		WCW_VertexPointer(3,GL_FLOAT,0,vbo->verts);
		WCW_NormalPointer(GL_FLOAT, 0, vbo->norms);
	}

	if (rdr_caps.use_vbos) {
		WCW_BindBuffer(GL_ARRAY_BUFFER_ARB, (int)rgbs);
		if (rdr_caps.chip & R200) {
			glColorPointer(3,GL_FLOAT,0,0); CHECKGL;
		} else {
			glColorPointer(4,GL_UNSIGNED_BYTE,0,0); CHECKGL;
		}
		WCW_BindBuffer(GL_ARRAY_BUFFER_ARB, vbo->vertex_array_id);
	} else {
		glColorPointer(4,GL_UNSIGNED_BYTE,0,rgbs); CHECKGL;
	}
}

static INLINEDBG void bindBufferNormals(VBO *vbo)
{
	if (modelBindBuffer(vbo))
	{
		if (vbo->sts2)
			texBindTexCoordPointer(TEXLAYER_GENERIC,vbo->sts2);
		texBindTexCoordPointer(TEXLAYER_BASE,vbo->sts);
		WCW_VertexPointer(3,GL_FLOAT,0,vbo->verts);
		WCW_NormalPointer(GL_FLOAT, 0, vbo->norms);
	}
}

#define MATINC			mat[3][2] += 0.00001; glMatrixMode(GL_MODELVIEW); mat43to44(mat,mat44); glLoadMatrixf((F32 *)mat44);

#define BIND1			bindBufferNormals(vbo);
#define BIND2			bindBufferNormals(vbo2);

#define DRAW1			glDrawElements(GL_TRIANGLES,elemcount,GL_UNSIGNED_INT,triptr);
#define DRAW2			glDrawElements(GL_TRIANGLES,elemcount2,GL_UNSIGNED_INT,triptr2);

#define SETTEXWHITE1	texSetWhite(0);
#define SETTEXGREY1		texBind(0, rdr_view_state.grey_tex_id);
#define SETTEXBLACK1	texBind(0, rdr_view_state.black_tex_id);

#define SETTEXWHITE2	texSetWhite(0); texSetWhite(1);
#define SETTEXGREY2		texBind(0, rdr_view_state.grey_tex_id); texBind(1, rdr_view_state.grey_tex_id);
#define SETTEXBLACK2	texBind(0, rdr_view_state.black_tex_id); texBind(1, rdr_view_state.black_tex_id);

#define SETTEXWHITE3	texSetWhite(0); texSetWhite(1); texSetWhite(2);
#define SETTEXGREY3		texBind(0, rdr_view_state.grey_tex_id); texBind(1, rdr_view_state.grey_tex_id); texBind(2, rdr_view_state.grey_tex_id);
#define SETTEXBLACK3	texBind(0, rdr_view_state.black_tex_id); texBind(1, rdr_view_state.black_tex_id); texBind(2, rdr_view_state.black_tex_id);

#define SETBLEND1 		modelBlendState(BlendMode(BLENDMODE_MULTIPLY, 0), FORCE_SET); WCW_ConstantCombinerParamerterfv(0, c); WCW_ConstantCombinerParamerterfv(1, c);
#define SETBLEND2 		modelBlendState(BlendMode(BLENDMODE_COLORBLEND_DUAL,0), FORCE_SET); WCW_ConstantCombinerParamerterfv(0, c); WCW_ConstantCombinerParamerterfv(1, c);

#define SETVP1			if (rdr_caps.use_vertshaders) {WCW_EnableVertexProgram(); WCW_BindVertexProgram(shaderMgrVertexPrograms[DRAWMODE_BUMPMAP_RGBS]);}
#define SETVP2			if (rdr_caps.use_vertshaders) {WCW_EnableVertexProgram(); WCW_BindVertexProgram(shaderMgrVertexPrograms[DRAWMODE_BUMPMAP_DUALTEX]);}



#define DOTEST8 { THETEST; THETEST; THETEST; THETEST; THETEST; THETEST; THETEST; THETEST; }
#define DOTEST4 { THETEST; THETEST; THETEST; THETEST; }


void rtPerfTest(RdrPerfInfo *info)
{
	static		int		timer;
	int			i,repeats=0,loopcount = g_perfio->loop_count;
	Mat4		mat;
	Mat44		mat44;

	RdrModel	*model = &info->models[0].model;
	VBO			*vbo = model->vbo;
	int			elemcount = model->vbo->tri_count*3;
	int			*triptr = model->vbo->tris;

	RdrModel	*model2 = &info->models[1].model;
	VBO			*vbo2 = model2 ? model2->vbo : 0;
	int			elemcount2 = vbo2 ? vbo2->tri_count*3 : 0;
	int			*triptr2 = vbo2 ? vbo2->tris : 0;

	float c[4] = {1, 1, 1, 1};

	assert(model);

	if (!timer)
		timer = timerAlloc();

	glClearColor( 0.0, 0.0, 0.0, 0.0 ); CHECKGL;
	glClearStencil(0); CHECKGL;
	glClearDepth(1); CHECKGL;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); CHECKGL;

	copyMat4(unitmat,mat);
	mat[3][1] = 0;
	mat[3][2] = -900;
	mat43to44(mat,mat44); 

	for (i = info->num_models-1; i >= 0; i--)
	{
		copyMat4(mat,info->models[i].model.mat);
		modelDrawDirect(&info->models[i].model);
	}


	memset( setBlendStateCalls, 0 , sizeof(setBlendStateCalls) );
	memset( setBlendStateChanges, 0 , sizeof(setBlendStateChanges) );
	memset( setDrawStateCalls, 0 , sizeof(setDrawStateCalls) );
	memset( setDrawStateChanges, 0 , sizeof(setDrawStateChanges) );

	g_perfio->trisDrawn = 0;

	timerStart(timer);

	SETTEXWHITE3

	while(timerElapsed(timer) < 2)
	{
		repeats++;
		switch(g_perfio->test_num)
		{
			xcase 0x0:
			{
				// repeated glDrawElements calls
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		DRAW1
					DOTEST8
					#undef THETEST
				}
				g_perfio->trisDrawn += model->vbo->tri_count * i;
			}

			xcase 0x1:
			{
				// switch objects
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  DRAW1  MATINC  BIND2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x2:
			{
				// switch fragment programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETBLEND1  DRAW1  MATINC  SETBLEND2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x3:
			{
				// switch objects and fragment programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETBLEND1  DRAW1  MATINC  BIND2  SETBLEND2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x4:
			{
				// switch vertex programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  DRAW1  MATINC  SETVP2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x5:
			{
				// switch objects and vertex programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  BIND1  DRAW1  MATINC  SETVP2  BIND2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x6:
			{
				// switch fragment programs and vertex programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  DRAW1  MATINC  SETVP2  SETBLEND2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x7:
			{
				// switch objects, fragment programs, and vertex programs
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  BIND1  DRAW1  MATINC  SETVP2  SETBLEND2  BIND2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}



			xcase 0x8:
			{
				// switch 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETTEXWHITE1  DRAW1  MATINC  SETTEXGREY1  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase 0x9:
			{
				// switch objects and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETTEXWHITE1  DRAW1  MATINC  BIND2  SETTEXGREY1  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xa:
			{
				// switch fragment programs and 1 texure
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETBLEND1  SETTEXWHITE1  DRAW1  MATINC  SETBLEND2  SETTEXGREY1  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xb:
			{
				// switch objects, fragment programs, and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETBLEND1  SETTEXWHITE1  DRAW1  MATINC  BIND2  SETBLEND2  SETTEXGREY1  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xc:
			{
				// switch vertex programs and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETTEXWHITE1  DRAW1  MATINC  SETVP2  SETTEXGREY1  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xd:
			{
				// switch objects, vertex programs, and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  BIND1  SETTEXWHITE1  DRAW1  MATINC  SETVP2  BIND2  SETTEXGREY1  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xe:
			{
				// switch fragment programs, vertex programs, and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  SETTEXWHITE1  DRAW1  MATINC  SETVP2  SETBLEND2  SETTEXGREY1  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0xf:
			{
				// switch objects, fragment programs, vertex programs, and 1 texture
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  BIND1  SETTEXWHITE1  DRAW1  MATINC  SETVP2  SETBLEND2  BIND2  SETTEXGREY1  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}




			xcase 0x10:
			{
				// switch 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETTEXWHITE2  DRAW1  MATINC  SETTEXGREY2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase 0x11:
			{
				// switch objects and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETTEXWHITE2  DRAW1  MATINC  BIND2  SETTEXGREY2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x12:
			{
				// switch fragment programs and 2 texures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETBLEND1  SETTEXWHITE2  DRAW1  MATINC  SETBLEND2  SETTEXGREY2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x13:
			{
				// switch objects, fragment programs, and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETBLEND1  SETTEXWHITE2  DRAW1  MATINC  BIND2  SETBLEND2  SETTEXGREY2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x14:
			{
				// switch vertex programs and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETTEXWHITE2  DRAW1  MATINC  SETVP2  SETTEXGREY2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x15:
			{
				// switch objects, vertex programs, and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  BIND1  SETTEXWHITE2  DRAW1  MATINC  SETVP2  BIND2  SETTEXGREY2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x16:
			{
				// switch fragment programs, vertex programs, and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  SETTEXWHITE2  DRAW1  MATINC  SETVP2  SETBLEND2  SETTEXGREY2  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x17:
			{
				// switch objects, fragment programs, vertex programs, and 2 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  BIND1  SETTEXWHITE2  DRAW1  MATINC  SETVP2  SETBLEND2  BIND2  SETTEXGREY2  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}



			xcase 0x18:
			{
				// switch 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETTEXWHITE3  DRAW1  MATINC  SETTEXGREY3  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase 0x19:
			{
				// switch objects and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETTEXWHITE3  DRAW1  MATINC  BIND2  SETTEXGREY3  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1a:
			{
				// switch fragment programs and 3 texures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETBLEND1  SETTEXWHITE3  DRAW1  MATINC  SETBLEND2  SETTEXGREY3  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1b:
			{
				// switch objects, fragment programs, and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  BIND1  SETBLEND1  SETTEXWHITE3  DRAW1  MATINC  BIND2  SETBLEND2  SETTEXGREY3  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1c:
			{
				// switch vertex programs and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETTEXWHITE3  DRAW1  MATINC  SETVP2  SETTEXGREY3  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1d:
			{
				// switch objects, vertex programs, and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  BIND1  SETTEXWHITE3  DRAW1  MATINC  SETVP2  BIND2  SETTEXGREY3  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1e:
			{
				// switch fragment programs, vertex programs, and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  SETTEXWHITE3  DRAW1  MATINC  SETVP2  SETBLEND2  SETTEXGREY3  DRAW1
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			xcase  0x1f:
			{
				// switch objects, fragment programs, vertex programs, and 3 textures
				for(i=0;i<loopcount;i+=8)
				{
					#define THETEST		MATINC  SETVP1  SETBLEND1  BIND1  SETTEXWHITE3  DRAW1  MATINC  SETVP2  SETBLEND2  BIND2  SETTEXGREY3  DRAW2
					DOTEST4;
					#undef THETEST
				}
				g_perfio->trisDrawn += (model->vbo->tri_count * i / 2) + (model2->vbo->tri_count * i / 2);
			}

			break;
		}

		windowUpdateDirect();
	}

	g_perfio->repeats = repeats;
	g_perfio->elapsed = timerElapsed(timer);
	windowUpdateDirect();
}

void rtPerfSet(RdrPerfIO **perfio)
{
	g_perfio = *perfio;
}
