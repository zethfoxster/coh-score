#define RT_ALLOW_BINDTEXTURE
#define RT_PRIVATE
#include "ogl.h"
#include "mathutil.h"
#include "rt_queue.h"
#include "rt_prim.h"
#include "wcw_statemgmt.h"
#include "rt_state.h"
#include "rt_tex.h"
#include "rt_stats.h"
#include "rt_pbuffer.h"
#include "cmdgame.h"

#define SET_COLOR(x)						\
	WCW_Color4_Force(	((x) & 0xff0000) >> 16,		\
						((x) & 0xff00) >> 8,		\
						(x) & 0xff,					\
						((x) & 0xff000000) >> 24);

void rdrSetFogDirect(RdrFog *vals)
{
	WCW_Fogf(GL_FOG_MODE,GL_LINEAR);
	if (rdr_caps.chip & NV1X) {
		glFogi(GL_FOG_DISTANCE_MODE_NV,GL_EYE_RADIAL_NV); CHECKGL;
	}
	WCW_Fogf(GL_FOG_START,vals->fog_dist[0]);  
	WCW_Fogf(GL_FOG_END,vals->fog_dist[1]);
	if (vals->fog_dist[0] > 1000000)
		WCW_Fog(0);
	else
		WCW_Fog(1);

	if (vals->has_color)
		WCW_FogColor(vals->fog_color);
}

void rdrSetFogDirect2(F32 near_dist, F32 far_dist, Vec3 color)
{
	RdrFog	vals;

	vals.fog_dist[0] = near_dist;
	vals.fog_dist[1] = far_dist;
	if (color)
	{
		vals.has_color = 1;
		copyVec3(color,vals.fog_color);
	}
	else
		vals.has_color = 0;	
	rdrSetFogDirect(&vals);
}

void rdrSetBgColorDirect(Vec3 bg_color)
{
	glClearColor(bg_color[0],bg_color[1],bg_color[2],1.f); CHECKGL;
}



void drawFilledBoxDirect(RdrLineBox *box)
{
	F32		z, x1, y1, x2, y2;
	U32		argb_ul, argb_ur, argb_ll, argb_lr;

	x1		= box->points[0][0];
	y1		= box->points[0][1];
	z		= box->points[0][2];
	x2		= box->points[1][0];
	y2		= box->points[1][1];
	argb_ul = box->colors[0];
	argb_ur	= box->colors[1];
	argb_ll	= box->colors[2];
	argb_lr	= box->colors[3];

	texSetWhite(TEXLAYER_BASE);

	glBegin(GL_QUADS);
	SET_COLOR(argb_ll);
	glVertex3f( x1,y1, z );
	SET_COLOR(argb_ul);
	glVertex3f( x1,y2, z );
	SET_COLOR(argb_ur);
	glVertex3f( x2,y2, z );

	//SET_COLOR(argb_ll);
	//glVertex3f( x1,y1, z );
	//SET_COLOR(argb_ur);
	//glVertex3f( x2,y2, z );
	SET_COLOR(argb_lr);
	glVertex3f( x2,y1, z );
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(2)
}

void drawLine2DDirect(RdrLineBox *line)
{
	texSetWhite(TEXLAYER_BASE);

	glBegin(GL_LINES);
	SET_COLOR(line->colors[0]);
	glVertex3fv(line->points[0] );
	SET_COLOR(line->colors[1]);
	glVertex3fv(line->points[1]);
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(1)
}

//#include "camera.h"
void drawLine3DDirect(RdrLineBox *line)
{
	// Draw.
	Vec3	a, b;
	Vec3	toCam;

	//subVec3(cam_info.cammat[3], line->points[0], toCam);
	subVec3(rdr_view_state.inv_viewmat[3], line->points[0], toCam);
	normalVec3(toCam);
	scaleVec3(toCam, 0.05, toCam);
	addVec3(line->points[0], toCam, a);

	//subVec3(cam_info.cammat[3], line->points[1], toCam);
	subVec3(rdr_view_state.inv_viewmat[3], line->points[1], toCam);
	normalVec3(toCam);
	scaleVec3(toCam, 0.05, toCam);
	addVec3(line->points[1], toCam, b);

	WCW_LoadModelViewMatrix(rdr_view_state.viewmat);

	texSetWhite(TEXLAYER_BASE);

	modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);

	WCW_Fog(0);
	glLineWidth( line->line_width ); CHECKGL;
	WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	WCW_PrepareToDraw();

	glBegin(GL_LINE_LOOP);
	SET_COLOR(line->colors[0]);
	glVertex3f(vecParamsXYZ(a));
	SET_COLOR(line->colors[1]);
	glVertex3f(vecParamsXYZ(b));
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(2)

	WCW_Fog(1);
}

void drawTriangle3DDirect(RdrLineBox *tri)
{
	// Draw.

	WCW_LoadModelViewMatrix(rdr_view_state.viewmat);

	texSetWhite(TEXLAYER_BASE);

	modelDrawState(DRAWMODE_COLORONLY, ONLY_IF_NOT_ALREADY);
	modelBlendState(BlendMode(BLENDMODE_MODULATE,0), ONLY_IF_NOT_ALREADY);

	WCW_Fog(0);
	WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	WCW_PrepareToDraw();

	glDisable(GL_CULL_FACE); CHECKGL;
	glBegin(GL_TRIANGLES);
	SET_COLOR(tri->colors[0]);
	glVertex3fv(tri->points[0]);
	SET_COLOR(tri->colors[1]);
	glVertex3fv(tri->points[1]);
	SET_COLOR(tri->colors[2]);
	glVertex3fv(tri->points[2]);
	glEnd(); CHECKGL;
	RT_STAT_DRAW_TRIS(1)
	glEnable(GL_CULL_FACE); CHECKGL;

	WCW_Fog(1);
}

static int get_texture = 0;

void rdrGetFrameBufferDirect(RdrGetFrameBuffer *get)
{
	PBuffer *pbuf = pbufGetCurrentDirect();
	rdrBeginMarker(__FUNCTION__);

//	if (rdr_caps.copy_subimage_from_rtt_invert_hack && pbufGetCurrentDirect() && !pbufGetCurrentDirect()->isFakedRTT) {
//		get->y = pbufGetCurrentDirect()->virtual_height - (get->y + get->height);
//	}

	if (pbuf && (pbuf->flags & PB_FBO) && pbuf->hardware_multisample_level > 1)
	{
		GLenum internalFormat;
		GLenum format;
		GLenum type;
		int color_texture = 0;
		int depth_texture = 0;

		if (get_texture == 0)
		{
			glGenTextures(1, &get_texture); CHECKGL;
		}

		WCW_EnableTexture( GL_TEXTURE_2D, 0 );
		WCW_BindTexture(GL_TEXTURE_2D, 0, get_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); CHECKGL;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); CHECKGL;

		switch(get->type)
		{
			case GETFRAMEBUF_RGB:
				internalFormat = GL_RGB8;
				format = GL_RGB;
				type = GL_UNSIGNED_BYTE;
				color_texture = get_texture;
				break;
			case GETFRAMEBUF_RGBA:
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				color_texture = get_texture;
				break;
			case GETFRAMEBUF_DEPTH:
			case GETFRAMEBUF_DEPTH_FLOAT:
			case GETFRAMEBUF_STENCIL:
				internalFormat = pbuf->depth_internal_format;
				format = pbuf->depth_format;
				type = pbuf->depth_type;
				depth_texture = get_texture;
				// TODO: convert from pbuf->depth_internal_format to desired type
				break;
			default :
				return;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, get->x + get->width, get->y + get->height, 0, format, type, get->data); CHECKGL;
		winCopyFrameBufferDirect(GL_TEXTURE_2D, color_texture, depth_texture, get->x, get->y, get->width, get->height, 0, false, false);
		WCW_BindTexture( GL_TEXTURE_2D, 0, get_texture );
		glGetTexImage(GL_TEXTURE_2D, 0, format, type, get->data); CHECKGL;
	}
	else
	{
		// Old code which can not work with multi-sampled render buffers
		switch(get->type)
		{
			xcase GETFRAMEBUF_RGB:
				glReadPixels(get->x,get->y,get->width,get->height,GL_RGB,GL_UNSIGNED_BYTE,get->data); CHECKGL;
			xcase GETFRAMEBUF_RGBA:
				glReadPixels(get->x,get->y,get->width,get->height,GL_RGBA,GL_UNSIGNED_BYTE,get->data); CHECKGL;
			xcase GETFRAMEBUF_DEPTH:
				glReadPixels(get->x,get->y,get->width,get->height,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,get->data); CHECKGL;
			xcase GETFRAMEBUF_STENCIL:
				glReadPixels(get->x,get->y,get->width,get->height,GL_STENCIL_INDEX,GL_UNSIGNED_INT,get->data); CHECKGL;
			xcase GETFRAMEBUF_DEPTH_FLOAT:
				glReadPixels(get->x,get->y,get->width,get->height,GL_DEPTH_COMPONENT,GL_FLOAT,get->data); CHECKGL;
		}
	}
	rdrEndMarker();
}

void rdrSetClipPlanesDirect(RdrClipPlane *rcp)
{
	WCW_EnableClipPlanes(rcp->numplanes, rcp->planes);
}

void rdrSetStencilDirect(RdrStencil *rs)
{
	if (rs->enable)
	{				
		WCW_EnableStencilTest();
		WCW_StencilFunc(GL_ALWAYS, rs->func_ref, ~0);
		if (rs->op == STENCILOP_KEEP)
			WCW_StencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		else if (rs->op == STENCILOP_INCR)
			WCW_StencilOp( GL_INCR, GL_INCR, GL_INCR );
	}
	else
	{
		WCW_DisableStencilTest();
	}
}

void rdrViewportDirect(Viewport *v)
{
	glViewport(v->x, v->y, v->width, v->height); CHECKGL;
	rdr_view_state.width = v->width;
	rdr_view_state.height = v->height;

	if (v->bNeedScissoring)
	{
		glEnable(GL_SCISSOR_TEST); CHECKGL;
		glScissor(v->x, v->y, v->width, v->height); CHECKGL;
	}
	else
	{
		glDisable(GL_SCISSOR_TEST); CHECKGL;
	}
}

void rdrClearScreenDirectEx(ClearScreenFlags clear_flags)
{
	int gl_clear_flags=0;
	rdrBeginMarker(__FUNCTION__);
	if (clear_flags & CLEAR_STENCIL) {
		gl_clear_flags |= GL_STENCIL_BUFFER_BIT;
		glClearStencil(0); CHECKGL;
	}
	if (clear_flags & CLEAR_DEPTH) {
		gl_clear_flags |= GL_DEPTH_BUFFER_BIT;
		if (clear_flags & CLEAR_DEPTH_TO_ZERO) {
			glClearDepth(0); CHECKGL;
		} else {
			glClearDepth(1); CHECKGL;
		}
	}
	if (clear_flags & CLEAR_COLOR)
		gl_clear_flags |= GL_COLOR_BUFFER_BIT;
	if (clear_flags & CLEAR_ALPHA) {
		glClearColor(0,0,0,0); CHECKGL;
	}
    glClear(gl_clear_flags); CHECKGL;
	rdrEndMarker();
}

void rdrClearScreenDirect()
{
	rdrClearScreenDirectEx(CLEAR_DEFAULT);
}

void rdrPerspectiveDirect(float *p)
{
	WCW_LoadProjectionMatrix(*(Mat44*)p);

	copyMat44(*(Mat44*)p, rdr_view_state.projectionMatrix);
	glMatrixMode(GL_MODELVIEW); CHECKGL;
}

void rdrDynamicQuad(RdrDynamicQuad* d)
{
	static int s_tex_id = 0;
	static int s_tex_width;
	static int s_tex_height;

	// if this is the first usage of the dynamic quad then create the texture
	// @todo this could be handled better but it is quick to implement
	if (!s_tex_id)
	{
		GLuint texture_id;
		GLenum error;

		glGenTextures(1, &texture_id); CHECKGL;
		WCW_BindTexture(GL_TEXTURE_2D, 0, texture_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); CHECKGL;

		gldGetError();
		glTexImage2D(GL_TEXTURE_2D, 0, 3, d->width_src,  d->height_src, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		error = gldGetError();

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );

		s_tex_id = texture_id;
		s_tex_width = d->width_src;
		s_tex_height = d->height_src;
	}

	// @todo not currently a very flexible system, texture always has to be the same size as
	// when first created
	onlydevassert( s_tex_width == d->width_src );
	onlydevassert( s_tex_height == d->height_src );

	WCW_BindTexture( GL_TEXTURE_2D, 0, s_tex_id );

	// refresh texture data if it has changed
	if (d->dirty && d->pixels)
	{
		glTexSubImage2D( GL_TEXTURE_2D, 0,
			0, 0,
			d->width_src, d->height_src,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			d->pixels );
	}

	glDisable(GL_CULL_FACE); CHECKGL;
	glLoadIdentity();
    glEnable( GL_TEXTURE_2D );
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_QUADS );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex2d( d->right_dst, d->bottom_dst );

        glTexCoord2f( 0.0f, 1.0f );
        glVertex2d( d->left_dst, d->bottom_dst );

        glTexCoord2f( 0.0f, 0.0f );
        glVertex2d( d->left_dst, d->top_dst );

        glTexCoord2f( 1.0f, 0.0f );
        glVertex2d( d->right_dst, d->top_dst );
    glEnd();
	glEnable(GL_CULL_FACE); CHECKGL;
}

