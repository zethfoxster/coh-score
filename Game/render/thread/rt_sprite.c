#define RT_PRIVATE
#define RT_ALLOW_BINDTEXTURE
#include "ogl.h"
#include "wcw_statemgmt.h"
#include "rt_sprite.h"
#include "rt_tex.h"
#include "mathutil.h"
#include "rt_state.h"
#include "rt_stats.h"
#include "SuperAssert.h"
#include "cmdgame.h"

static void setConstantColor(U32 color1,U32 color2)
{
	int		i;

	for(i=0;i<4;i++)
	{
		constColor0[i] = ((color1 >> (8*i)) & 255) * 1.f / 255.f;
		constColor1[i] = ((color2 >> (8*i)) & 255) * 1.f / 255.f;
	}
	WCW_ConstantCombinerParamerterfv(0, constColor0);
	WCW_ConstantCombinerParamerterfv(1, constColor1);
}

static INLINEDBG void bindVerticesInline(RdrSpriteVertex *verts)
{
	WCW_BindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	WCW_BindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	WCW_VertexPointer(3, GL_FLOAT, sizeof(RdrSpriteVertex), &verts->point);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(RdrSpriteVertex), &verts->rgba); CHECKGL;
	texBindTexCoordPointerEx(TEXLAYER_BASE, sizeof(RdrSpriteVertex), &verts->texcoord);
}

static INLINEDBG int drawSpritesInline(RdrSpritesCmd *cmd, int offset)
{
	int vertex_count = cmd->sprite_count * 4;
	GLubyte *texels = NULL;

	rdrBeginMarker(__FUNCTION__);

	RT_STAT_ADDSPRITE(cmd->sprite_count * 2)

	//assert(cmd->state.target == GL_TEXTURE_2D);
	if( cmd->state.target == GL_TEXTURE_2D)
		texBind(TEXLAYER_BASE, cmd->state.texid);
	else {
		// must be cubemap, do some work here to extract desired face image as 2D texture for display
		int cubewidth = 0;
		static int id = 0;

		assert(cmd->state.target >=GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB && cmd->state.target<=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB);
		
		WCW_BindTexture(GL_TEXTURE_CUBE_MAP, TEXLAYER_BASE, cmd->state.texid);
		glGetTexLevelParameteriv(cmd->state.target, 0, GL_TEXTURE_WIDTH, &cubewidth); CHECKGL;
		assert(cubewidth > 0);

		// allocate texels so can copy cubemap face image data
		texels = (GLubyte *)malloc(cubewidth * cubewidth * 4);  assert(texels);
		memset(texels, 0, cubewidth * cubewidth * 4);
		glGetTexImage(cmd->state.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels); CHECKGL;
		WCW_BindTexture(GL_TEXTURE_CUBE_MAP, 0, 0);

		if( !id ) {
			glGenTextures(1, &id); CHECKGL;
			texBind(TEXLAYER_BASE, id);

			WCW_ActiveTexture(TEXLAYER_BASE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); CHECKGL;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECKGL;
		} else {
			texBind(TEXLAYER_BASE, id);
		}

		WCW_ActiveTexture(TEXLAYER_BASE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cubewidth, cubewidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels); CHECKGL;
	}	

	if (cmd->state.use_scissor)
	{
		glEnable(GL_SCISSOR_TEST); CHECKGL;
		glScissor(cmd->state.scissor_x, cmd->state.scissor_y, cmd->state.scissor_width, cmd->state.scissor_height); CHECKGL;
	}
	else
	{
		glDisable(GL_SCISSOR_TEST); CHECKGL;
	}

	if (cmd->state.additive)
	{
		WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	WCW_PrepareToDraw();

	glDrawArrays(GL_QUADS, offset, vertex_count); CHECKGL;

	// free cubemap face texels if allocated above

	RT_STAT_DRAW_TRIS(cmd->sprite_count * 2)

	rdrEndMarker();

	return vertex_count;
}

void drawSpriteDirect(RdrSpritesCmd *cmd)
{
	RdrSpriteVertex		*sprite_verts = (RdrSpriteVertex *) (cmd+1);

	rdrBeginMarker(__FUNCTION__);
	glDisable(GL_DEPTH_TEST); CHECKGL;

	WCW_DepthMask(0);
	
	WCW_EnableClientState(GLC_VERTEX_ARRAY);
	WCW_EnableClientState(GLC_COLOR_ARRAY);
	
	texEnableTexCoordPointer(TEXLAYER_BASE, true);
	bindVerticesInline(sprite_verts);
	drawSpritesInline(cmd, 0);

	WCW_DisableClientState(GLC_VERTEX_ARRAY);
	WCW_DisableClientState(GLC_COLOR_ARRAY);
	texEnableTexCoordPointer(TEXLAYER_BASE, false);

	if (cmd->state.use_scissor) {
		glDisable(GL_SCISSOR_TEST); CHECKGL;
	}

	if (cmd->state.additive)
	{
		WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	WCW_DepthMask(1);
	rdrEndMarker();
}


#ifndef FINAL
// Sprite debugging tools

// utility to log data for a given sprite cmd and its verts to stdout
static void dbg_log_sprite(RdrSpritesCmd* aCmd, RdrSpriteVertex* verts, int index)
{
	int i;
	int vertex_count = aCmd->sprite_count * 4;

	// this is the one we are interested in, log information about this sprite
	printf("%04d ------\n quads: %d, verts: %d, tex: %d\n", index, aCmd->sprite_count, vertex_count, aCmd->state.texid);
	for (i=0; i<vertex_count;++i)
	{
		RdrSpriteVertex* vert = verts + i;
		if (i%4==0)
			printf("  quad: %02d\n", i/4);
		printf("   vert: %d\n", i );
		printf("    pos: %f, %f, %f\n", vert->point[0],vert->point[1], vert->point[2]);
		printf("    tc: %f, %f\n", vert->texcoord[0],vert->texcoord[1]);
		printf("     c: %d, %d, %d, %d\n", vert->rgba[0],vert->rgba[1],vert->rgba[2],vert->rgba[3]);
	}
}

static INLINEDBG void dbg_render_single_sprite_from_package( RdrSpritesPkg* pkg, int index )
{
	RdrSpritesCmd	*cmd = (RdrSpritesCmd *) (pkg+1);
	RdrSpriteVertex *verts = (RdrSpriteVertex *) (((char *)cmd) + (pkg->sprite_cmd_count * sizeof(RdrSpritesCmd)));
	int j, offset;

	onlydevassert( index >= 0 && index < pkg->sprite_cmd_count );

	// first determine the offset of the vertex data for this particular cmd
	offset = 0;
	for (j = 0; j < index; j++)
	{
		RdrSpritesCmd *aCmd = cmd + j;
		int vertex_count = aCmd->sprite_count * 4;
		offset += vertex_count;
	}

	// now draw the requested cmd
	drawSpritesInline(cmd + index, offset);
	// log the sprite, @todo should we put a debug flag to control this as an option?
	dbg_log_sprite( cmd + index, verts + offset, index );
}

// utility to display and log a particular index of the sprite batch list
// returns true if dbg sprite has been processed and walking the full list
// should be disabled.
static INLINEDBG int dbg_a_particular_sprite( RdrSpritesPkg* pkg )
{
	if ( g_client_debug_state.this_sprite_only < 0 )
		return false;	// dbg state not active

	// render and log only the target, if it is in bounds
	if (g_client_debug_state.this_sprite_only < pkg->sprite_cmd_count)
	{
		dbg_render_single_sprite_from_package(pkg, g_client_debug_state.this_sprite_only);
	}

	return true;	// debug mode engaged, don't render the full sprite batch
}

#endif // !FINAL

void drawAllSpritesDirect(RdrSpritesPkg *pkg)
{
	RdrSpritesCmd	*cmd = (RdrSpritesCmd *) (pkg+1);
	RdrSpriteVertex *verts = (RdrSpriteVertex *) (((char *)cmd) + (pkg->sprite_cmd_count * sizeof(RdrSpritesCmd)));

	rdrBeginMarker(__FUNCTION__);

	if (pkg->test_depth) {
		glEnable(GL_DEPTH_TEST); CHECKGL;
	} else {
		glDisable(GL_DEPTH_TEST); CHECKGL;
	}

	if (!pkg->write_depth)
 		WCW_DepthMask(0);

	WCW_EnableClientState(GLC_VERTEX_ARRAY);
	WCW_EnableClientState(GLC_COLOR_ARRAY);
	texEnableTexCoordPointer(TEXLAYER_BASE, true);
	bindVerticesInline(verts);

#ifndef FINAL
	if ( !dbg_a_particular_sprite(pkg) )	// are we debugging a single sprite from the package?
#endif
	{
		// draw the entire package of sprites
		int j, offset = 0;
		for (j = 0; j < pkg->sprite_cmd_count; j++)
		{
			offset += drawSpritesInline(cmd + j, offset);
		}
	}

	WCW_DisableClientState(GLC_VERTEX_ARRAY);
	WCW_DisableClientState(GLC_COLOR_ARRAY);
	texEnableTexCoordPointer(TEXLAYER_BASE, false);

	glDisable(GL_SCISSOR_TEST); CHECKGL;
	WCW_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// reset these
	if (!pkg->write_depth)
		WCW_DepthMask(1);
	rdrEndMarker();
}

