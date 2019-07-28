#include "tex.h"
#include "ogl.h"
#include "rt_tex.h"
#include "rt_queue.h"
#include "renderprim.h"
#include "tex_gen.h"
#include "SuperAssert.h"
#include "MemoryMonitor.h"
#include "StringCache.h"
#include "osdependent.h"
#include "mathutil.h"	// for pow2
#include "rt_init.h"	// for rdr_caps

static const char TEXGEN_MEMMONITOR_NAME[] = "tex_gen";

int getImageByteCount(int pixel_format, int width, int height)
{
	if (pixel_format == GL_LUMINANCE_ALPHA)
		return width * height * 2;

	if (pixel_format == GL_RGB)
		return width * height * 3;

	if (pixel_format == GL_RGBA)
		return width * height * 4;

	if (pixel_format == GL_RGBA8)
		return width * height * 4;

	if (pixel_format == GL_DEPTH_COMPONENT)
		return width * height * 3;

	if (pixel_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		return ((width+3)/4)*((height+3)/4)*8;

	if (pixel_format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || pixel_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		return ((width+3)/4)*((height+3)/4)*16;

	assertmsg(0, "Unknown pixel format passed to getImageByteCount!");

	return 0;
}

int getImageByteCountWithMips(int pixel_format, int width, int height, int mipcount)
{
	int ret=0;
	while (mipcount>=0) {
		ret += getImageByteCount(pixel_format, width, height);
		width >>= 1;
		height >>= 1;
		if (!width)
			width = 1;
		if (!height)
			height = 1;
		mipcount--;
	}
	return ret;
}

// no dst_format needed when updating a sub region
void texGenUpdateRegion(BasicTexture *bind, U8 *bitmap, int x, int y, int width, int height, int pixel_format)
{
	RdrSubTexParams	rtex = {0};

	// N.B. The texture must have had a full update or been initialized with a glTexImage2D call to setup
	// the true texture dimensions before you can operate on a sub region.
	// If not the glTexSubImage2D call will ultimately fail with a GL_INVALID_OPERATION
	// For the case of the texGen textures we can use the load_state[0] flag to see if this has happened yet
	onlydevassert( bind->load_state[0] == TEX_LOADED );

	rtex.id			= bind->id;
	rtex.target		= bind->target;
	rtex.x_offset	= x;
	rtex.y_offset	= y;
	rtex.width		= width;
	rtex.height		= height;

	rtex.format = pixel_format;

	// uncompressed only
	assertmsg(pixel_format == GL_RGB || pixel_format == GL_RGBA || pixel_format == GL_LUMINANCE_ALPHA, "Bad pixel format passed to texGenUpdateRegion!");

	rdrTexSubCopy(&rtex, bitmap, getImageByteCount(pixel_format, width, height));

	// memory usage for the full sized texture (not the sub region we are operating on) should have
	// been tracked during the full update to define the texture dimensions
}

void texGenUpdateEx(BasicTexture *bind, U8 *bitmap, int pixel_format, int mipcount)
{
	int new_mem_usage;
	RdrTexParams	rtex = {0};

	rtex.width		= bind->realWidth; // bind->width;
	rtex.height		= bind->realHeight;// bind->height;
	rtex.id			= bind->id;
	rtex.target		= bind->target;

	rtex.src_format = pixel_format;

	if (pixel_format == GL_RGB)
		rtex.dst_format = GL_RGB8;
	else if (pixel_format == GL_RGBA)
		rtex.dst_format = GL_RGBA8;
	else if (pixel_format == GL_LUMINANCE_ALPHA)
		rtex.dst_format = GL_LUMINANCE8_ALPHA8;
	else if (pixel_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
		rtex.dst_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		rtex.dxtc = 1;
	} else
		rtex.dst_format = pixel_format;

	if (mipcount) {
		rtex.mip_count = mipcount;
		rtex.mipmap = 1;
	}

	rtex.clamp_s	= rtex.clamp_t		= 1;
	bind->load_state[0] = TEX_LOADED;
	new_mem_usage = getImageByteCountWithMips(pixel_format, rtex.width, rtex.height, mipcount);
	rdrTexCopy(&rtex, bitmap, new_mem_usage);
	if (new_mem_usage!=bind->memory_use[0]) {
		memMonitorTrackUserMemory(TEXGEN_MEMMONITOR_NAME, 1, bind->memory_use[0]?MOT_REALLOC:MOT_ALLOC, new_mem_usage - bind->memory_use[0]);
		bind->memory_use[0] = new_mem_usage;
	}
}

// This version will check if we had to grow the real texture to satisfy power of 2 requirements.
// It will correctly handle what is necessary to do the initial texture define and sub region update
void texGenUpdateLogical(BasicTexture *bind, U8 *bitmap, int pixel_format, int mipcount)
{
	if (bind->width != bind->realWidth || bind->height != bind->realHeight)
	{
		// First update? If so we need to define the texture extent first with a full update

		// N.B. Since we've had to grow the texture beyond its logical size the client will probably only want to 
		// update the sub region of the texture that is actually in use. However, before we can use sub region
		// updates this texture must have had a full size update to get initialized with a glTexImage2D call.
		// This will setup the true texture dimensions before you can operate on a sub region.
		// If not the glTexSubImage2D call will ultimately fail with a GL_INVALID_OPERATION
		if ( bind->load_state[0] == TEX_LOADING )
		{
			// @todo could alternatively have a 'define' render thread call which calls
			// glTexImage2D() but with NULL data so that we don't have to make a dummy allocation
			// here.
			int bitmap_size = getImageByteCount(pixel_format, bind->realWidth,  bind->realHeight);
			U8 *bitmap = (U8*)malloc(bitmap_size);
			if (bitmap)
			{
				memset(bitmap, 0, bitmap_size);
				// This does the initial glTexImage2D call on the texture and set state to TEX_LOADED
				texGenUpdateEx(bind, bitmap, pixel_format, mipcount);
				free(bitmap);
			}
		}

		// now do a sub region update to just update the logical texture
		texGenUpdateRegion(bind, bitmap, 0, 0, bind->width, bind->height, pixel_format);
	}
	else
	{
		// logical is the same as real size so just do a full update
		texGenUpdateEx(bind, bitmap, pixel_format, mipcount);
	}
}

void texGenUpdateFromScreen(BasicTexture *bind, int x, int y, int width, int height, bool depth_buffer)
{
	int new_mem_usage;
	RdrTexParams	rtex = {0};

	if (bind->width == width && bind->height == height && bind->load_state[0] == TEX_LOADED)
		rtex.sub_image = 1;
	rtex.width		= bind->width = width;
	rtex.height		= bind->height = height;
	rtex.x			= x;
	rtex.y			= y;
	rtex.id			= bind->id;
	rtex.target		= bind->target;
	rtex.from_screen = 1;
	rtex.point_sample = depth_buffer?1:0;

	rtex.dst_format = depth_buffer?GL_DEPTH_COMPONENT:GL_RGBA8;

	rtex.clamp_s	= rtex.clamp_t		= 1;
	bind->load_state[0] = TEX_LOADED;
	rdrTexCopy(&rtex, 0, 0);
	new_mem_usage = getImageByteCount(rtex.dst_format, rtex.width, rtex.height);
	if (new_mem_usage!=bind->memory_use[0]) {
		memMonitorTrackUserMemory(TEXGEN_MEMMONITOR_NAME, 1, bind->memory_use[0]?MOT_REALLOC:MOT_ALLOC, new_mem_usage - bind->memory_use[0]);
		bind->memory_use[0] = new_mem_usage;
	}
}


void texGenUpdate(BasicTexture *bind, U8 *bitmap)
{
	texGenUpdateEx(bind, bitmap, GL_RGBA, 0);
}

BasicTexture *texGenNew(int width,int height, char *name)
{
	BasicTexture *basicBind = calloc(sizeof(*basicBind),1);
	char buf[1024];

	sprintf(buf, "AUTOGEN_TEX - %s", name);
	

	basicBind->id = rdrAllocTexHandle();
	basicBind->target = GL_TEXTURE_2D;
	basicBind->actualTexture = basicBind;

	// mark texture as 'loading' moves to loaded on first 'update' of texture contents
	// I think that needs to happen before it gets used or texDemandLoad will choke thinking
	// this is a bind it needs to try and load.
	// @todo
	// not sure if there is good debugging for that case or not
	basicBind->load_state[0] = TEX_LOADING;
	basicBind->gloss = 1;
	basicBind->name = allocAddString(buf);
	basicBind->dirname = "";
	basicBind->memory_use[0] = 0;

	// Setup texture dimensions appropriately for hardware

	// - If texture has power of 2 dimensions then all hardware can handle it.
	// - Hardware that supports GL_ARB_texture_non_power_of_two transparently handles any dimension textures.
	//   Introduced roughly for generations: GeForce 6, Radeon 9600, Intel 965 .
	if ( rdr_caps.supports_arb_np2 || (width == pow2(width) && height == pow2(height)) )
	{
		basicBind->width = basicBind->realWidth = width;
		basicBind->height = basicBind->realHeight = height;
	}
	else
	{
		// Non Power of Two texture (NPOT) without explicit hardware support.
		// If we are requesting a dynamic NPOT texture then we will need
		// to be careful how we handle it.
		// Older video hardware can either only render with power of two dimension texture
		// or it will support a special NPOT texture mode through the GL_ARB_texture_rectangle extension.
		// GL_ARB_texture_rectangle requires special setup to use (e.g., texture coordinates are in pixels and not normalized).

		// For simplicity we choose not to provide support for GL_ARB_texture_rectangle at this time.
		// We make the *real* texture size a power of 2 and the rendering system will adjust texture coordinates as
		// necessary. We also need to make sure that we only update the logical subregion of the texture.
		basicBind->width = width;
		basicBind->height = height;
		basicBind->realWidth = pow2(width);
		basicBind->realHeight = pow2(height);
	}

	return basicBind;
}

void texGenFree(BasicTexture *bind)
{
	memMonitorTrackUserMemory(TEXGEN_MEMMONITOR_NAME, 1, MOT_FREE, bind->memory_use[0]);
	bind->memory_use[0] = 0;
	rdrTexFree(bind->id);

	// Mac only until we're sure this is stable.
	if (bind->mipid && IsUsingCider())
		rdrTexFree(bind->mipid);

	//free(bind->name); using a string cache, and there is a lot of reuse so little threat of a leak here
	free(bind);
}

