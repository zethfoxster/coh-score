#ifndef _GROUPSCENE_H
#define _GROUPSCENE_H

typedef struct
{
	char	*src;
	char	*dst;
} TexSwap;

typedef struct
{
	Vec2	match_hue;
	Vec3	hsv;
	F32		magnify;
	U8		reset;
} ColorSwap;

typedef struct DOFValues
{
	F32		focusDistance;
	F32		focusValue;
	F32		nearValue;
	F32		nearDist;
	F32		farValue;
	F32		farDist;
} DOFValues;

typedef struct
{
	char			*cubemap;
	char			**sky_names;
	int				clip_fogged_geometry;
	F32				fog_ramp_color;
	F32				fog_ramp_distance;
	F32				scale_specularity;
	TexSwap			**tex_swaps;
	TexSwap			**material_swaps;
	ColorSwap		**color_swaps;
	F32				maxHeight;
	F32				minHeight;
	F32				fallingDamagePercent;
	Vec3			force_fog_color;
	Vec3			force_ambient_color;
	F32				force_fog_near;
	F32				force_fog_far;
	DOFValues		force_dof_values;
	F32				force_dof_far_dist;
	Vec3			character_min_ambient_color;
	Vec3			character_ambient_scale;
	int				colorFogNodes; // Apply the force_fog_color to fog nodes
	int				doNotOverrideFogNodes;
	F32				toneMapWeight; // How much tonemapping to do in HDR effects
	F32				bloomWeight; // How much glow/bloom to do in HDR effects
	F32				desaturateWeight; // How much to desaturate the scene
	F32				indoorLuminance; // Luminance value to use while indoors
	int				stippleFadeMode; // Do stipple fading.  Turns off stencil shadows.
	F32				sky_fade_rate; // Rate at which we fade between sky files
	F32				ambientOcclusionWeight;
	int				shadowMaps_Off;	// boolean
	F32				shadowMaps_modulationClamp_ambient;
	F32				shadowMaps_modulationClamp_diffuse;
	F32				shadowMaps_modulationClamp_specular;
	F32				shadowMaps_modulationClamp_vertex_lit;
	F32				shadowMaps_modulationClamp_NdotL_rolloff;
	F32				shadowMaps_lastCascadeFade;
	F32				shadowMaps_farMax;
	F32				shadowMaps_splitLambda;
	F32				shadowMaps_maxSunAngleDeg;
} SceneInfo;

extern SceneInfo scene_info;
void sceneLoad(char *fname);


#endif