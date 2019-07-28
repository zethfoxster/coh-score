#define RT_ALLOW_BINDTEXTURE
#define RT_PRIVATE
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_model.h"
#include "rt_shadow.h"
#include "rt_state.h"
#include "rt_tex.h"
#include "rt_tricks.h"
#include "rt_stats.h"
#include "cmdgame.h"
#include "mathutil.h"
#include "splat.h"

typedef struct ShadowInfo
{
	// shadowing tricks many to throw away
	Model   *shadow;
	Vec3	*shadow_verts;
	int		shadow_vert_count;
	Vec3	*shadow_norms;
	int		*shadow_tris;
	int		shadow_tri_count;
	int		zero_area_tri_count;
	Vec3	*tri_norms;
	U8		*open_edges;
} ShadowInfo;


//###################################################################
//######### Stencil Shadows #########################################

//##### Create Shadow Volume From Flat Plane
static void create_stipple_pattern(int *pat, int alpha)
{
	int		y;
	U32		pa,pb,pc,pd;

	if (alpha < 16)
	{
		pa = 0x0;
		pb = 0x0;
		pc = 0x11111111;	// 1
		pd = 0x0;
	}
	else if (alpha < 32)
	{
		pa = 0x44444444;	//   4
		pb = 0x0;
		pc = 0x11111111;	// 1
		pd = 0x0;
	}
	else if (alpha < 48)
	{
		pa = 0x55555555;	// 1 4
		pb = 0x0;
		pc = 0x11111111;	// 1
		pd = 0x0;
	}
	else if (alpha < 64)
	{
		pa = 0x55555555;	// 1 4
		pb = 0x0;
		pc = 0x55555555;	// 1 4
		pd = 0x0;
	}
	else if (alpha < 80)
	{
		pa = 0x55555555;	// 1 4
		pb = 0x22222222;	//  2
		pc = 0x55555555;	// 1 4
		pd = 0x0;
	}
	else if (alpha < 96)
	{
		pa = 0x55555555;	// 1 4
		pb = 0x22222222;	//  2
		pc = 0x55555555;	// 1 4
		pd = 0x88888888;	//    8
	}
	else if (alpha < 102)
	{
		pa = 0x55555555;	// 1 4
		pb = 0xaaaaaaaa;	//  2 8
		pc = 0x55555555;	// 1 4
		pd = 0x88888888;	//    8
	}
	else if (alpha < 128)
	{
		pa = 0x55555555;	// 1 4
		pb = 0xaaaaaaaa;	//  2 8
		pc = 0x55555555;	// 1 4
		pd = 0xaaaaaaaa;	//  2 8
	}
	else if (alpha < 144)
	{
		pa = 0x55555555;	// 1 4
		pb = 0xeeeeeeee;	//  248
		pc = 0x55555555;	// 1 4
		pd = 0xaaaaaaaa;	//  2 8
	}
	else if (alpha < 160)
	{
		pa = 0x55555555;	// 1 4
		pb = 0xeeeeeeee;	//  248
		pc = 0x55555555;	// 1 4
		pd = 0xbbbbbbbb;	// 12 8
	}
	else if (alpha < 176)
	{
		pa = 0x55555555;	// 1 4
		pb = -1;			// 1248
		pc = 0x55555555;	// 1 4
		pd = 0xbbbbbbbb;	// 12 8
	}
	else if (alpha < 192)
	{
		pa = 0x55555555;	// 1 4
		pb = -1;			// 1248
		pc = 0x55555555;	// 1 4
		pd = -1;			// 1248
	}
	else if (alpha < 208)
	{
		pa = 0x55555555;	// 1 4
		pb = -1;			// 1248
		pc = 0xdddddddd;	// 1 48
		pd = -1;			// 1248
	}
	else if (alpha < 224)
	{
		pa = 0x77777777;	// 124
		pb = -1;			// 1248
		pc = 0xdddddddd;	// 1 48
		pd = -1;			// 1248
	}
	else if (alpha < 240)
	{
		pa = -1;			// 1248
		pb = -1;			// 1248
		pc = 0xdddddddd;	// 1 48
		pd = -1;			// 1248
	}
	else
	{
		pa = -1;
		pb = -1;
		pc = -1;
		pd = -1;
	}

	for (y = 0; y < 32; y+=4)
	{
		pat[y] = pa;
		pat[y+1] = pb;
		pat[y+2] = pc;
		pat[y+3] = pd;
	}
}

static U32 stipple_patterns[16][32];
static void make_stipples(void)
{
	static bool stipple_inited = false;
	int		i;
	if (stipple_inited)
		return;
	stipple_inited = true;

	for(i=0;i<16;i++)
		create_stipple_pattern(stipple_patterns[i],i * 16);
}

U8 *getStipplePatternSimple(int alpha)
{
	make_stipples();
	return (U8*)stipple_patterns[alpha >> 4];
}

static U32 random_stipple[256][32];
static void init_random_stipple(void)
{
	int i;
	U16 offs[1024];
	int num_offs=1024;
	for (i=0; i<1024; i++) 
		offs[i] = i;
	for (i=0; i<256; i++) {
		// Fill in ith place with 4 more pixels
		int j;
		if (i!=0) {
			memcpy(random_stipple[i], random_stipple[i-1], sizeof(random_stipple[i]));
		} else {
			memset(random_stipple[i], 0, sizeof(random_stipple[i]));
		}
		for (j=0; j<4; j++) {
			int k = rand() * num_offs / (RAND_MAX + 1);
			int v = offs[k];
			offs[k] = offs[--num_offs];
			SETB(random_stipple[i], v);
		}
		// For second half inverted, do i <128, and this:
		//for (j=0; j<32; j++) {
		//	random_stipple[255 - i][j] = (-1)^random_stipple[i][j];
		//}
	}
}

void make_random_stipples(void)
{
	static bool stipple_inited = false;
	if (stipple_inited)
		return;
	stipple_inited = true;
	init_random_stipple();
}

U8 *getStipplePattern(int alpha, bool bInvert)
{
	U32 *ret;
	if (bInvert) {
		alpha = 255 - alpha;
	}
//	make_random_stipples();
//	ret = random_stipple[alpha];
	make_stipples();
	ret = stipple_patterns[alpha >> 4];
	if (bInvert) {
		static U32 pat[32];
		int y;
		for (y = 0; y < 32; y++)
			pat[y] = ret[y]^-1;
		ret = pat;
	}
	return (U8*)ret;
}

void shadowFreeDirect(ShadowInfo *shadow)
{
	free(shadow->shadow_verts);
	free(shadow->shadow_norms);
	free(shadow->shadow_tris);
	free(shadow->tri_norms);
	free(shadow->open_edges);
	free(shadow);
}

//Formerly GlueVerts
static void initializeShadowInfo(VBO *vbo)
{
	int		i,j,count=0,tcount;
	F32		*v;
	ShadowInfo	*shadow;

	vbo->shadow = calloc(sizeof(*vbo->shadow),1);
	shadow = vbo->shadow;

	tcount = vbo->tri_count * 3;

	//1. Build shadow->shadow_vert, shadow_tri, shadow_norms, and tri_norms 

	shadow->shadow_verts	= calloc(vbo->vert_count,sizeof(Vec3));
	shadow->shadow_norms	= calloc(vbo->vert_count,sizeof(Vec3));
	shadow->shadow_tris		= calloc(tcount, sizeof(int));
	shadow->tri_norms		= calloc(vbo->tri_count,sizeof(Vec3));
	
	for(i=0;i<tcount;i++)
	{
		//get next vert in tri
		v = vbo->verts[vbo->tris[i]];

		//If the vert is unique, write it to shadow_verts (and it's norm to shadow_norms)
		//This gets rid of already encountered verts, and 
		//verts close to other verts for texturing reasons or whatever.
		//Unique or not, write the new array position of this vert to shadow_tris so there's
		//an accurate tri list for these verts
		for(j=0;j<count;j++)
		{
			if (nearSameVec3Tol(v,shadow->shadow_verts[j],0.000005f))
				break;
		}
		if (j >= count)
		{
			copyVec3(v,shadow->shadow_verts[count]);
			copyVec3(vbo->norms[vbo->tris[i]],shadow->shadow_norms[count]);
			count++;
		}

		shadow->shadow_tris[i] = j;

		//For each tri, calculate the face normal.
		if ((i % 3) == 2)
		{
		Vec3	tvec1,tvec2;
		int		*idx = shadow->shadow_tris + i - 2;

			subVec3(shadow->shadow_verts[idx[1]],shadow->shadow_verts[idx[0]],tvec1);
			subVec3(shadow->shadow_verts[idx[2]],shadow->shadow_verts[idx[1]],tvec2);
			crossVec3(tvec1,tvec2,shadow->tri_norms[i/3]);
			normalVec3(shadow->tri_norms[i/3]);
		}
	}
	shadow->shadow_vert_count = count;
	shadow->shadow_tri_count = vbo->tri_count;

	//2. Build shadow->open_edges.  Search through all the triangle edges and compare then to all
	//other triangle edges and see if they are the same.  If no match, then this is an open
	//edge.  Then something with checking co-planarness that I don't get.

	shadow->open_edges		= calloc(vbo->tri_count,1);
	memset(shadow->open_edges,0,vbo->tri_count);

	for(i=0;i<tcount;i++)
	{
	int		idx,jdx,idx1,idx2;

		idx1 = shadow->shadow_tris[i];
		idx = i+1;
		if ((idx % 3) == 0)
			idx -= 3;
		idx2 = shadow->shadow_tris[idx];

		for(j=0;j<tcount;j++)
		{
			if (j/3 == i/3)
				continue;
			jdx = j+1;
			if ((jdx % 3) == 0)
				jdx -= 3;
			if (idx2 == shadow->shadow_tris[j] && idx1 == shadow->shadow_tris[jdx])
			{
				if (nearSameVec3Tol(shadow->tri_norms[i/3],shadow->tri_norms[j/3],0.001f))
					break;
			}
		}
		if (j >= tcount)
			shadow->open_edges[i/3] |= 1 << (i % 3);
	}

	if (0)
		memset(shadow->open_edges,7,tcount/3); //???
	
}


#define addtris(a,b,c) (tris[tri_count + 0] = a, tris[tri_count + 1] = b, tris[tri_count + 2] = c, tri_count += 3)

/*Create a shadow volume from a flat plane
uses globals cam_info
*/
static void setupVolume(Vec3 vbuf[], int tris[], int * tri_count_ptr, VBO *vbo,Mat4 mat,F32 dist,Vec3 offsetx)
{
	int		i,t,count,vcount;
	Vec3	dir;
	F32		d;
	int		do_stretch = 0;
	int		tri_count = 0;
	ShadowInfo	*shadow;

	copyVec3(offsetx,dir);
	normalVec3(dir);
	shadow = vbo->shadow;

	//Make a copy of this shape at each end of extrusion (near in off by a bit to prevent some self-shadow)
	for(i=0;i<shadow->shadow_vert_count;i++)
		addVec3(offsetx,shadow->shadow_verts[i],vbuf[shadow->shadow_vert_count+i]);

	scaleVec3(dir,0.1,offsetx);
	for(i=0;i<shadow->shadow_vert_count;i++)
		addVec3(offsetx,shadow->shadow_verts[i],vbuf[i]);

	//Sew up edges
	vcount = shadow->shadow_vert_count;
	count = shadow->shadow_tri_count * 3;

	tri_count = 0;
	for(t=i=0;i<count;i+=3,t++)
	{
	int		a0,b0,c0,a1,b1,c1;

		d = dotVec3(dir,shadow->tri_norms[t]);
		a0 = shadow->shadow_tris[i+0];
		a1 = vcount + a0;
		b0 = shadow->shadow_tris[i+1];
		b1 = vcount + b0;
		c0 = shadow->shadow_tris[i+2];
		c1 = vcount + c0;


		if (d >= 0)
		{
			addtris(c0,b0,a0);
			addtris(a1,b1,c1);

			if (shadow->open_edges[t] & 1)
			{
				addtris(a0,b0,b1);
				addtris(b1,a1,a0);
			}

			if (shadow->open_edges[t] & 2)
			{
				addtris(b0,c0,c1);
				addtris(c1,b1,b0);
			}

			if (shadow->open_edges[t] & 4)
			{
				addtris(c0,a0,a1);
				addtris(a1,c1,c0);
			}
		}
		else
		{
			addtris(a0,b0,c0);
			addtris(c1,b1,a1);

			if (shadow->open_edges[t] & 1)
			{
				addtris(b1,b0,a0);
				addtris(a0,a1,b1);
			}

			if (shadow->open_edges[t] & 2)
			{
				addtris(c1,c0,b0);
				addtris(b0,b1,c1);
			}

			if (shadow->open_edges[t] & 4)
			{
				addtris(a1,a0,c0);
				addtris(c0,c1,a1);
			}
		}
	}
	//vert_count = vcount * 2;
	*tri_count_ptr = tri_count;
}

//##End Flat Plane Volume builder######################################
//################################################################

/*render a simple volume 
uses globals curr_draw_state and shadow_tri_count
*/
static void drawVolume(Vec3 vbuf[], int tris[], int tri_count)
{
	WCW_VertexPointer(3,GL_FLOAT,0,vbuf);
	
	WCW_PrepareToDraw();

	glDrawElements(GL_TRIANGLES,tri_count,GL_UNSIGNED_INT,tris); CHECKGL;
	RT_STAT_DRAW_TRIS(tri_count/3) // tri_count is a horrible lie.  Whoever named it should be ashamed.  It is actually the index count.

	if(0){
		int		i;
		F32		*v;

		modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
		modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);

		WCW_Color4(255,255,255,255);

		for(i=0;i<tri_count;i+=3)
		{
			glBegin(GL_LINE_LOOP);
			v = vbuf[tris[i]];
			glVertex3f(v[0],v[1],v[2]);

			v = vbuf[tris[i+1]];
			glVertex3f(v[0],v[1],v[2]);

			v = vbuf[tris[i+2]];
			glVertex3f(v[0],v[1],v[2]);
			glEnd(); CHECKGL;
			RT_STAT_DRAW_TRIS(3)
		}
	}
	//*/
}

debugDrawShadowVolume(Vec3 vbuf[], int tris[], int tri_count, Mat4 mat, int alpha, int shadow_mask)
{
	if (rdr_view_state.shadowvol == 1)
	{
		glEnable(GL_CULL_FACE); CHECKGL;
		WCW_Color4(100,255,0,70);
		//glCullFace(GL_NONE); CHECKGL; //you can't actually say this, you have to use glDisable
		glDisable(GL_CULL_FACE); CHECKGL;
		drawVolume(vbuf, tris, tri_count);
		glEnable(GL_CULL_FACE); CHECKGL;
		//glCullFace(GL_FRONT); CHECKGL;
		//tri_count -= shadow->zero_area_tri_count* 3;
		//drawVolume(vbuf, tris, tri_count);
		//WCW_Color4(0,0,255,70);
		//glCullFace(GL_BACK); CHECKGL;
		//drawVolume(vbuf, tris, tri_count);
		//tri_count += shadow->zero_area_tri_count* 3;
		//return;
	}
	if (rdr_view_state.shadowvol  == 3)
	{
		glFrontFace(GL_CCW); CHECKGL;
		glEnable(GL_CULL_FACE); CHECKGL;
		WCW_Color4(255,255,255,150);
		glCullFace(GL_BACK); CHECKGL;
		drawVolume(vbuf, tris, tri_count);
		glFrontFace(GL_CW); CHECKGL;
		//return;
	}
	if (rdr_view_state.shadowvol == 4)
	{
		glEnable(GL_CULL_FACE); CHECKGL;
		WCW_Color4(0,0,255,70);
		glCullFace(GL_BACK); CHECKGL;
		drawVolume(vbuf, tris, tri_count);
		//return;
	}
}

/*Given an extruded shadow volume, render it to the stencil buffer twice.
vbuf, tris, tri_count, mat = the extruded shadow volume's verts, tri's, tri count, and mat
alpha = this modelect's shadow intensity for which stipple to use (only to keep shadows from popping out at distance)
shadow_mask = glStencilFunc ref value (look up)
*/

static void stencilShadowVolume(Vec3 vbuf[], int tris[], int tri_count, Mat4 mat, int alpha, int shadow_mask)
{
	Mat44		matx;

	mat43to44(mat,matx);
	WCW_LoadModelViewMatrixM44(matx);

	modelDrawState(DRAWMODE_FILL, ONLY_IF_NOT_ALREADY);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);

	texSetWhite(TEXLAYER_BASE);
	texSetWhite(TEXLAYER_GENERIC);

	modelBindDefaultBuffer();     

	{
		static U8 chunk[100000];
		static int init = 0;
		if( !init ) 
		{
			memset( chunk, 1, 100000 );
			init = 1;
		}
		glColorPointer(4,GL_UNSIGNED_BYTE,0,chunk); CHECKGL;
	}

	if( rdr_view_state.shadowvol ) //debug
	 	debugDrawShadowVolume(vbuf, tris, tri_count, mat, alpha, shadow_mask);

	//##### Set the stencil state parameters
	glDisable(GL_CULL_FACE); CHECKGL;
	WCW_EnableGL_LIGHTING(1);
	WCW_Light( GL_LIGHT0, GL_DIFFUSE, matx[3] );
	WCW_LightPosition( matx[3], mat );

	if (alpha != 255)
	{
		// TODO: Remove this once ATI memory leak regarding glEnable(GL_POLYGON_STIPPLE) is fixed (11/06/09)
		if (!(rdr_caps.chip & R200) || game_state.ati_stipple_leak)
		{
			glEnable(GL_POLYGON_STIPPLE); CHECKGL;
			glPolygonStipple(getStipplePatternSimple(alpha)); CHECKGL;
		}
	}

	WCW_EnableStencilTest();
	glStencilMask(~0); CHECKGL;

	if (!shadow_mask) {
		glStencilFunc(GL_ALWAYS, 0, ~0); CHECKGL;
	} else {
		glStencilFunc(GL_NOTEQUAL, shadow_mask, shadow_mask); CHECKGL;
	}
	glColorMask(0,0,0,0); CHECKGL;
	WCW_DepthMask(0);
	glDepthFunc(GL_LESS); CHECKGL;
	glEnable(GL_CULL_FACE); CHECKGL;

	//##### Draw front faces of shadow volume, incrementing stencil.
	glCullFace(GL_FRONT); CHECKGL;
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP); CHECKGL;
	drawVolume(vbuf, tris, tri_count);

	//##### Draw back faces of shadow volume, decrementing stencil.
	glCullFace(GL_BACK); CHECKGL;
	glStencilOp(GL_KEEP, GL_DECR,GL_KEEP); CHECKGL;// GL_DECR);
	drawVolume(vbuf, tris, tri_count);

	//##### Unset the stencil parameters
	glCullFace(GL_BACK); CHECKGL;
	glColorMask(1,1,1,1); CHECKGL;
	WCW_DepthMask(1);
	if (game_state.iAmAnArtist && !game_state.edit && !game_state.wireframe) { // Exagerate z-fighting issues
		glDepthFunc((global_state.global_frame_count&1)?GL_LEQUAL:GL_LESS); CHECKGL;
	} else {
		glDepthFunc(GL_LEQUAL); CHECKGL;
	}
	WCW_DisableStencilTest();
	glDisable(GL_POLYGON_STIPPLE); CHECKGL;

	// reset this so the drawstate is correct
	WCW_EnableGL_LIGHTING(0);
}

void modelDrawShadowVolumeDirect(RdrStencilShadow *rs)
{
	static Vec3	vbuf[10000];
	static int	tribuf[10000];
	static int	tri_count;
	VBO			*vbo;

	vbo = rs->vbo;

	RT_STAT_ADDMODEL(vbo);

	//If this model had already been set up as a VBO object, you can't extrude it, so forget it.
	//I would put up a data error, but artists won't understand it, and Im in a hurry, and I think
	//the stencils are going the way of the dodo anyway.
	if( vbo->element_array_id )
		return;

	if ( !vbo->shadow )
		initializeShadowInfo( vbo );

	RT_STAT_ADDMODEL(vbo);

	setupVolume(vbuf, tribuf, &tri_count, vbo, rs->mat, rs->dist, rs->offsetx);

	//###### Do Stenciling
	stencilShadowVolume(vbuf, tribuf, tri_count, rs->mat, rs->alpha, rs->shadow_mask); 
}


static void drawtex()
{
	F32	x1,x2,y1,y2;
	Vec4 shadowcolor;

	copyVec4(rdr_view_state.shadowcolor, shadowcolor);

	modelDrawState(DRAWMODE_FILL, FORCE_SET);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0),FORCE_SET);
	
	WCW_PrepareToDraw();
	
	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;
	//Don't do anything that calls statemanager, cuz I'm inside a pushattrib
	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho( 0.0f, 640, 0.0f, 480, -1.0f, 1.0f ); CHECKGL;
	glMatrixMode( GL_MODELVIEW ); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glDisable(GL_LIGHTING); CHECKGL;
	glDisable( GL_FOG ); CHECKGL;

	WCW_BindTexture( GL_TEXTURE_2D, 0, rdr_view_state.white_tex_id );
	
	//WCW_Color4(0,0,0,50); //mm temp 
	WCW_Color4(shadowcolor[0] * 255, shadowcolor[1] * 255, shadowcolor[2] * 255, shadowcolor[3] * 255); //mm temp 
	//modelSetAlpha(200);  //mm temp 
	x1 = 0;
	x2 = 640;
	y2 = 0;
	y1 = 480;
	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0 );
	glVertex2f( x1, y1 );
	glTexCoord2f( 1, 0 );
	glVertex2f( x2, y1 );
	glTexCoord2f( 1, 1 );
	glVertex2f( x2, y2 );
	glTexCoord2f( 0, 1 );
	glVertex2f( x1, y2 );
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(2)

	glPopMatrix(); CHECKGL;
	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPopMatrix(); CHECKGL;
	glPopAttrib(); CHECKGL;
	glMatrixMode( GL_MODELVIEW ); CHECKGL;

	WCW_FetchGLState();
	WCW_ResetState();

	texSetWhite(0);
}

void shadowFinishSceneDirect()
{ 
	const FogContext *old_fog_context;
	FogContext dummy_fog_context;

    /* Only draw where stencil is non-zero,
       ie. only in the shadowed regions. */
	WCW_EnableStencilTest();

    WCW_DepthMask(0);
	glDepthFunc(GL_ALWAYS); CHECKGL;

	glStencilFunc(GL_NOTEQUAL, 0, ~(128|64)); CHECKGL;
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); CHECKGL;

	// Save the fog state because drawtex() calls WCW_ResetState() which will lose the old fog min/max values
	old_fog_context = WCW_GetFogContext();
	dummy_fog_context = *old_fog_context;
	WCW_SetFogContext(&dummy_fog_context);

	drawtex();

	// Restore the old fog state
	WCW_SetFogContext((FogContext*)old_fog_context);

	WCW_DepthMask(1);
	if (game_state.iAmAnArtist && !game_state.edit && !game_state.wireframe) { // Exagerate z-fighting issues
		glDepthFunc((global_state.global_frame_count&1)?GL_LEQUAL:GL_LESS); CHECKGL;
	} else {
		glDepthFunc(GL_LEQUAL); CHECKGL;
	}

	WCW_DisableStencilTest();
}
// ##########################################################################
// ############ End Stencil Shadows


void rdrSetupStippleStencilDirect(void)
{
	int i;

	modelDrawState(DRAWMODE_FILL, ONLY_IF_NOT_ALREADY);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0),ONLY_IF_NOT_ALREADY);
	glPushAttrib(GL_ALL_ATTRIB_BITS); CHECKGL;
	//Don't do anything that calls statemanager, cuz I'm inside a pushattrib
	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glOrtho( 0.0f, 1, 0.0f, 1, -1.0f, 1.0f ); CHECKGL;
	glMatrixMode( GL_MODELVIEW ); CHECKGL;
	glPushMatrix(); CHECKGL;
	glLoadIdentity(); CHECKGL;
	glDisable(GL_LIGHTING); CHECKGL;
	glDisable( GL_FOG ); CHECKGL;
	WCW_BindTexture( GL_TEXTURE_2D, 0, rdr_view_state.white_tex_id );

	// TODO: Remove this once ATI memory leak regarding glEnable(GL_POLYGON_STIPPLE) is fixed (11/06/09)
	if (!(rdr_caps.chip & R200) || game_state.ati_stipple_leak) {
		glEnable(GL_POLYGON_STIPPLE); CHECKGL;
	}
	glDepthFunc(GL_ALWAYS); CHECKGL;
	glDepthMask(0); CHECKGL;

	glStencilMask(~0); CHECKGL;
	glStencilFunc(GL_ALWAYS, 0, 0); CHECKGL;
	glStencilOp(GL_KEEP, GL_INCR, GL_INCR); CHECKGL;
	glEnable(GL_STENCIL_TEST); CHECKGL;
	glColorMask(0, 0, 0, 0); CHECKGL;

	for (i=0; i<256; i+=16) {
		U8 *pat = getStipplePattern(i, false);
		glPolygonStipple(pat); CHECKGL;
		glBegin( GL_QUADS );
		glTexCoord2f( 0, 0 );
		glVertex2f( 0, 1 );
		glTexCoord2f( 1, 0 );
		glVertex2f( 1, 1 );
		glTexCoord2f( 1, 1 );
		glVertex2f( 1, 0 );
		glTexCoord2f( 0, 1 );
		glVertex2f( 0, 0 );
		glEnd(); CHECKGL;
		RT_STAT_DRAW_TRIS(2)
	}
	glPopMatrix(); CHECKGL;
	glMatrixMode( GL_PROJECTION ); CHECKGL;
	glPopMatrix(); CHECKGL;
	glMatrixMode( GL_MODELVIEW ); CHECKGL;
	glPopAttrib(); CHECKGL;
	texSetWhite(0);

	WCW_FetchGLState();
	WCW_ResetState();

	if (0) {
		unsigned char buf[32*32];
		int j;
		glReadPixels(0, 0, 32, 32, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, buf); CHECKGL;
		for (i=0; i<32; i++) {
			for (j=0; j<29; j++) {
				printf("%3d ", buf[j + i*32]);
			}
			printf("\n");
		}
		printf("");
	}
}


void modelDrawShadowObjectDirect( Splat * splat )
{
	int		tri_bytes,vert_bytes,st_bytes,color_bytes,splatShadowsDrawn;
	U8		*mem;
	F32		viewspace_z;

	tri_bytes	= splat->triCnt * sizeof(splat->tris[0]);
	vert_bytes	= splat->vertCnt * sizeof(splat->verts[0]);
	st_bytes	= splat->vertCnt * sizeof(splat->sts[0]);
	color_bytes	= splat->vertCnt * 4 * sizeof(splat->colors[0]);

	mem = (U8*)(splat + 1);

	splat->tris = (void *)mem;
	mem += tri_bytes;

	splat->verts = (void*)mem;
	mem += vert_bytes;

	splat->sts = (void*)mem;
	mem += st_bytes;

	splat->sts2 = (void*)mem;
	mem += st_bytes;

	splat->colors = (void*)mem;
	mem += color_bytes;

	viewspace_z = *((F32 *)mem);
	mem += sizeof(F32);

	splatShadowsDrawn = *((int *)mem);

	if( splat->stAnim )
		animateSts( splat->stAnim, NULL ); 
	//epsilon = -1.0 * ((ZPULL_DELTA+(PER_SHADOW_OFFSET*splatShadowsDrawn) )  * ((-1)/pz) ) ; 
	//set a nice offset so the z pull gives no fighting
	//TO DO : add offset = (F32)splatShadowsDrawn / 1000.0;
	if (0)
	{
		Mat44 projectionMat44;
		F32 pz,f,n,delta;
		F32 epsilon;

#define EXTRYBIT 0.01
#define MYCONST 0.0001
#define ZPULL_DELTA 0.005
#define PER_SHADOW_OFFSET 0.0001
 
		pz	= MIN( -20, viewspace_z );  //should be a negative z number   
		n	= rdr_view_state.nearz;
		f	= rdr_view_state.farz;
 
		delta =  EXTRYBIT + ( MYCONST * pz * pz )   /   ( 1.0f + MYCONST * pz ); //plus a small number? 
 
		epsilon = ( -2.0f * f * n  * delta )   /   ( ( f + n ) * pz * (pz + delta ) );  
 
		//epsilon/=2;    
 
	 	memcpy( projectionMat44, rdr_view_state.projectionMatrix, sizeof( projectionMat44 ) ); 
		projectionMat44[2][2] *= 1.0 + epsilon;        
 
#if RDRMAYBEFIX
		if( game_state.simpleShadowDebug )
		{
			xyprintf( 30, 30, "pz %f   f %f   n %f   MYCONST %f  EXTRABIT %f  PMatBefore %f", pz, f, n, MYCONST, EXTRYBIT, rdr_view_state.projectionMatrix[2][2] );  
			xyprintf( 30, 31, "delta   %f    epsilon   %f   PMatAfter %f", delta, epsilon, projectionMat44[2][2] ); 
		}
#endif
		glMatrixMode( GL_PROJECTION ); CHECKGL;
		glPushMatrix(); CHECKGL;
		glLoadMatrixf( (GLfloat *)projectionMat44 ); CHECKGL;
	}

	//Set ModelView Matrix: Verts are in world space, so just use the camera viewmat fixed up a little
	if (0)
	{
		Mat4 m;
		Mat4 base;
		F32 offset;

		copyMat4( unitmat, base );  

		offset = (F32)splatShadowsDrawn / 500.0;  //Move shadow up from surface to prevent z fighting  
		scaleVec3( splat->normal, (0.01 + offset), base[3] ); //Add a little to each shadow drawn this frame so shadows don't z fight each other.  

		mulMat4( rdr_view_state.viewmat, base, m );           
		m[3][2] += 0.02 + offset; //move it toward the camera a little to reduce z fighting for vertical surfaces

		WCW_LoadModelViewMatrix( m );  
	}
	else
	{
		WCW_LoadModelViewMatrix( rdr_view_state.viewmat );  

	}

	glEnable(GL_POLYGON_OFFSET_FILL); CHECKGL;
	glPolygonOffset(-1,-1); CHECKGL;

	modelDrawState(DRAWMODE_DUALTEX, ONLY_IF_NOT_ALREADY); 
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);
	modelBindDefaultBuffer();
	setupFixedFunctionVertShader(NULL);
	
	if (game_state.texoff)
	{
		texSetWhite(TEXLAYER_BASE);
		texSetWhite(TEXLAYER_GENERIC);
	}
	else
	{
		texBind(TEXLAYER_BASE, splat->texid1); 
		texBind(TEXLAYER_GENERIC, splat->texid2);
	}

	if( splat->flags & SPLAT_ADDITIVE )  
	{
		WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
		} else {
			WCW_FogPush(0);
		}
	}

	if (splat->flags & SPLAT_SUBTRACTIVE)
	{
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_REVERSE_SUBTRACT_EXT); CHECKGL;
			WCW_BlendFuncPush(GL_SRC_ALPHA, GL_ONE);
		} else {
			// this does not subtract, but fakes it by masking
			WCW_BlendFuncPush(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		}
		// Need to disable fog on additive
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
		} else {
			WCW_FogPush(0);
		}
	}
	
	WCW_DepthMask(0);
	glColorPointer(4,GL_UNSIGNED_BYTE,0,splat->colors); CHECKGL;
	texBindTexCoordPointer( TEXLAYER_BASE, splat->sts ); 
	texBindTexCoordPointer( TEXLAYER_GENERIC, splat->sts2 ); 
	WCW_VertexPointer(3,GL_FLOAT,0, splat->verts );      
	glCullFace( GL_BACK ); CHECKGL; //it this right?
	//glDisable(GL_CULL_FACE); CHECKGL; //is this right?
	
	WCW_PrepareToDraw();

	glDrawElements(GL_TRIANGLES, splat->triCnt * 3, GL_UNSIGNED_INT, splat->tris ); CHECKGL;
	RT_STAT_DRAW_TRIS(splat->triCnt)

	glDisable(GL_POLYGON_OFFSET_FILL); CHECKGL;

	if( splat->flags & SPLAT_ADDITIVE )
	{
		WCW_BlendFuncPop();
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			curr_draw_state = -1;
			curr_blend_state.intval = -1;
			//glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			//since we didn't record which state glFinalCombinerInputNV was coming from when we
			//set the TRICK_ADDITIVE, for now whatever comes next needs to reset the blend state
			//to make sure it's got hte right one
		} else
			WCW_FogPop();
	}

	if( splat->flags & SPLAT_SUBTRACTIVE )
	{
		WCW_BlendFuncPop();
		if (rdr_caps.supports_subtract) {
			glBlendEquationEXT(GL_FUNC_ADD_EXT); CHECKGL;
		}
		if ((rdr_caps.chip & NV1X) && !(rdr_caps.chip & ARBFP)) {
			curr_draw_state = -1;
			curr_blend_state.intval = -1;
			//glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_RGB); CHECKGL;
			//since we didn't record which state glFinalCombinerInputNV was coming from when we
			//set the TRICK_ADDITIVE, for now whatever comes next needs to reset the blend state
			//to make sure it's got hte right one
		} else
			WCW_FogPop();
	}

	//Stop the spinning
	{
		WCW_LoadTextureMatrixIdentity(0);
		WCW_LoadTextureMatrixIdentity(1);
	} 

	WCW_DepthMask(1);
	//glCullFace( GL_BACK ); CHECKGL;
	if(0)
	{
		glMatrixMode( GL_PROJECTION ); CHECKGL;
		glPopMatrix(); CHECKGL;
	}
	WCW_LoadModelViewMatrixIdentity();  
}
