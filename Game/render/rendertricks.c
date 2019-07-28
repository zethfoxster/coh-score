#include "ogl.h"
#include "rt_state.h"
#include "rendertricks.h"
#include "camera.h"
#include "cmdcommon.h"
#include "wcw_statemgmt.h"
#include "renderutil.h"
#include "cmdgame.h"
#include "font.h"
#include "tricks.h"
#include "tex.h"

static F32 cam_yaw_sin,cam_yaw_cos;
static  Mat4 camfacemat = {{-1.0,0,0},{0,1.0,0},{0,0,-1.0},{0.0,0.0,0.0}};

//Used for scrolling textures in the shaders


void gfxNodeSetAlpha(GfxNode *node,U8 alpha,int root)
{
	for(;node;node = node->next)
	{
		node->alpha = alpha;
	
		if (node->child)
			gfxNodeSetAlpha(node->child,alpha,0);
		if (root)  //note this wackiness that means only this and all its children get called, and not the root's kids
			break;
	}
}

//Used by fx
void gfxNodeSetAlphaFx(GfxNode *node,U8 alpha, int root)
{
	for(;node;node = node->next)
	{
		node->alpha = alpha;
		//node->flags |= GFXNODE_MAXALPHA;

		if (node->child)
			gfxNodeSetAlphaFx(node->child, alpha, 0);
		if (root)
			break;
	}
}


void QuickYawMat(Mat3 mat)
{
	int		i;
	F32		t;

	for(i=0;i<3;i++)
	{ 
		t		  = mat[0][i]*cam_yaw_cos - mat[2][i]*cam_yaw_sin;
		mat[2][i] = mat[2][i]*cam_yaw_cos + mat[0][i]*cam_yaw_sin;
		mat[0][i] = t; 
	}
}


