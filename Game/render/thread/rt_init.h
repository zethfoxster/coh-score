#ifndef _RT_INIT_H
#define _RT_INIT_H

#ifdef RT_PRIVATE
#include "rt_queue.h"
#endif

// "chipset"/hw capability/generation identifier bit flags (used with rdr_caps.chip)
// These flags are set based on vendor and hw capability and are used to select
// appropriate rendering path options and shader variants.
// In general mulitiple flags will be set for a specific vendor. For example, on
// an R430 class chip the R200 and R300 bits will also be set.
//
// Wikipedia has good tables of the various chipset generations. For example:
// http://en.wikipedia.org/wiki/Comparison_of_Nvidia_graphics_processing_units
// http://en.wikipedia.org/wiki/Comparison_of_AMD_graphics_processing_units

// IMPORTANT NOTE: The shader manager passes on the bits set in the chip field
// as defines when compiling shaders. So, if you change the order or add
// assignments that will probably invalidate the md5 values in the shader cache
// and force the shaders to recompile. That's not necessarily a problem but it
// is something to be aware of. See initShaderBuildDescriptor()

enum
{
	//
	// Using OpenGL 'register combiner' logic (also forces no vertex programs)
	// If this is set no other flags will be set
	TEX_ENV_COMBINE = (1 << 0), 

	//
	// Vendor specific chipset flags

	// nVidia
	NV1X = (1 << 1),		// nVidia GeForce 2 class and later
	NV2X = (1 << 2),		// nVidia GeForce 3 class and later (same era R200)
	NV3X = (1 << 3), 		// nVidia GeForce 5 series and later, also know as GeForce FX (same era R300)
	NV4X = (1 << 4),		// nVidia GeForce 6 series and later

	// ATI/AMD
	R200 = (1 << 8),		// ATI Radeon 8500/9000/9200 class and later, DX8 card, (same era NV2X)
	R300 = (1 << 9),		// ATI Radeon 9700/9800 class and later, DX9 card, (same era NV3X)
	R430 = (1 << 10),		// ATI Radeon X800 class and later (same era NV4X)

	//
	// Standard GL capabilities flags

	// These bits will be set if the hardware supports the OpenGL standard ARB programmable shaders
	// These paths will generally be preferred over vendor specific paths
	ARBVP = (1 << 16),
	ARBFP = (1 << 17),

	GLSL = (1 << 18),		// GLSL capable 
	DX10_CLASS = (1 << 19),	// DX10 class and later

};

typedef enum GfxFeatures { // Bitmask for what features are enabled/allowed
	GFXF_WATER                      = 1 << 0,
	GFXF_BLOOM                      = 1 << 1,
	GFXF_TONEMAP                    = 1 << 2,
	GFXF_MULTITEX                   = 1 << 3, // Any of our more complicated multi-texture schemes (required for water?)
	GFXF_MULTITEX_DUAL              = 1 << 4, // Dual-materialed multitex 
	GFXF_HQBUMP                     = 1 << 5, // "high quality" bumpmapping
	GFXF_MULTITEX_HQBUMP            = 1 << 6, // "high quality" bumpmapping on multitex shaders
	GFXF_FPRENDER                   = 1 << 7, // Render to a floating point buffer for Bloom, etc
	GFXF_WATER_DEPTH                = 1 << 8,
	GFXF_DOF                        = 1 << 9,
	GFXF_BUMPMAPS                   = 1<< 10,
	GFXF_BUMPMAPS_WORLD             = 1<< 11,
	GFXF_DESATURATE                 = 1<< 12,
	GFXF_AMBIENT                    = 1<< 13,
	GFXF_CUBEMAP					= 1<< 14,
	GFXF_SHADOWMAP	                = 1<< 15,
	GFXF_WATER_REFLECTIONS          = 1<< 16,
	// MANUALLY update this enum
	GFXF_NUM_FEATURES = 17
} GfxFeatures;

typedef struct
{
	U32		filled : 1;
	U32		use_vertshaders : 1;
	U32		use_fixedfunctionfp : 1;
	U32		use_fixedfunctionvp : 1;
	U32		use_vbos : 1;
	U32		bad_card_or_driver : 1;
	U32		rgbbuffers_as_floats_hack : 1; // For old ATI drivers
	U32		use_occlusion_query : 1;
	U32		supports_arb_np2:1;
	U32		partially_supports_arb_np2:1;
	U32		supports_arb_rect:1;
	U32		use_pbuffers:1;			// Even if this is 0, we still use pbuffers for cursors
	U32		supports_pbuffers:1;	// unless not supported
	U32		supports_fbos:1;
	U32		supports_pixel_format_float:1;
	U32		supports_pixel_format_float_mipmap:1;
	U32		supports_render_texture:1;
	U32		supports_subtract:1;
	U32		supports_multisample:1;
	U32		supports_ext_framebuffer_multisample:1;
	U32		supports_ext_framebuffer_blit:1;
	U32		supports_anisotropy:1;
	U32		supports_packed_stencil:1;
	U32		supports_occlusion_query:1;
	U32		supports_arb_shadow:1;
	U32		supports_cg_fp40:1;
	U32		supports_direct_state_access:1;
	U32		supports_vp_clip_planes_nv:1;			// profile is available for which user clip planes will work in vertex programs (e.g., skinning)
	U32		copy_subimage_from_rtt_invert_hack:1;	// For (all/old?) ATI drivers
	U32		copy_subimage_shifted_from_fbos_hack:1; // For ATI drivers older than the .6000 debug driver we got
	U32		disable_stencil_shadows:1;
	U32		limited_fragment_instructions:1;
	U32		filled_in:1;				// Whether or not the rdr_caps structure has been filled in yet
	int		num_texunits;				// Maximum number of simultaneously bound textures the hardware supports
	int		max_layer_texunits;			// Max texture units that can be used by material specs on this HW platform
	int		num_texcoords;				// How many texture coord/mat sets can be passed
	int		max_texture_size;			// from GL_MAX_TEXTURE_SIZE: largest texture that the GL can handle
	int		max_samples;				// for supports_multisample
	int		max_fbo_samples;			// for supports_ext_framebuffer_multisample
	int		chip;						// standards rendering caps and chipset class bit flags (e.g. ARBFB, NV4X)
	int		maxTexAnisotropic;
	int		max_program_local_parameters;		// from GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB
	int		max_program_env_parameters;			// from GL_MAX_PROGRAM_ENV_PARAMETERS_ARB
	int		max_program_native_attribs;			// from GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB
	int		max_program_native_instructions;	// from GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB
	int		max_program_native_temporaries;		// from GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB
	GfxFeatures	features;				// features in use (not all allowed features need to be in use, e.g. user prefs)
	GfxFeatures	allowed_features;		// allowed features as determined by HW capabilities and command line flags, etc.
	char	gl_vendor[128];				// GL_VENDOR string of GL context (so we can report it in error reports and db logs)
	char	gl_renderer[128];			// GL_RENDERER string of GL context (so we can report it in error reports and db logs)
	char	gl_version[128];			// GL_VERSION string of GL context (so we can report it in error reports and db logs)
} RenderCaps;

extern RenderCaps	rdr_caps;

#ifdef RT_PRIVATE
void rdrInitOnce(void);
void rdrInitDirect(void);
void rdrPerspectiveDirect(float *p);
void rdrViewportDirect(Viewport *v);
void rdrClearScreenDirect(void);
void rdrClearScreenDirectEx(ClearScreenFlags clear_flags);
void rdrInitExtensionsDirect(void);
void rdrInitOnceDirect(void);
void reloadShaderCallbackDirect(const char *relpath);
void rebuildCachedShadersCallbackDirect( const char* relpath);
#endif

#endif
