#ifndef _RT_UTIL_H
#define _RT_UTIL_H

void rt_InitDebugTuningMenuItems(void);

// material/shader toggles
#ifndef FINAL
typedef struct
{
	U32		flag_enable:1;	// this is the overall enable to load/use debug shaders
	U32		flag_loading:1;	// set during the call to the shader manager to load
	U32		flag_loaded:1;	// this is set when the debug shaders have been loaded once
	U32		flag_use_debug_shaders:1;	// want to be able to toggle debug shaders quickly w/o dismissing the gfx HUD

	// corresponds to ui grid rows
	U32		use_vertex_color;		// material
	U32		use_blend_base;
	U32		use_blend_dual;
	U32		use_blend_multiply;
	U32		use_blend_addglow;
	U32		use_blend_alpha;
	U32		use_lod_alpha;

	U32		use_ambient;			// lighting
	U32		use_diffuse;
	U32		use_specular;
	U32		use_light_color;
	U32		use_gloss_factor;
	U32		use_bump;
	U32		use_shadowed;			

	U32		use_fog;				// feature
	U32		use_reflection;
	U32		use_sub_material_1;
	U32		use_sub_material_2;
	U32		use_material_building;
//	U32		use_material;
//	U32		use_faux_reflection;

	U32		mode_texture_display:1;		// texture display mode
	U32		tex_display_rgba:1;
	U32		tex_display_rgb:1;
	U32		tex_display_alpha:1;

	U32		selected_texture;

	U32		mode_visualize:1;			// visualization mode
	U32		selected_visualization;
	U32		visualize_tex_id;			// texture to aid visualization (e.g. uv layout)
} ShaderDebugState;

extern ShaderDebugState g_mt_shader_debug_state;

#endif


#endif
