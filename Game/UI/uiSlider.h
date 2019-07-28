#ifndef UISLIDER_H
#define UISLIDER_H

#include "uiInclude.h"

typedef enum SliderType
{
	SLIDER_SCALE,		// indicates the slider type
	SLIDER_BONE_SCALE,
	SLIDER_CHARACTER_HEAD_SCALE,
	SLIDER_CHARACTER_SHOULDER_SCALE,
	SLIDER_CHARACTER_CHEST_SCALE,
	SLIDER_CHARACTER_WAIST_SCALE,
	SLIDER_CHARACTER_HIP_SCALE,
	SLIDER_CHARACTER_LEG_SCALE,
	SLIDER_CHARACTER_ARM_SCALE,

	SLIDER_CHARACTER_HEADX_SCALE,
	SLIDER_CHARACTER_HEADY_SCALE,
	SLIDER_CHARACTER_HEADZ_SCALE,
	SLIDER_CHARACTER_BROWX_SCALE,
	SLIDER_CHARACTER_BROWY_SCALE,
	SLIDER_CHARACTER_BROWZ_SCALE,
	SLIDER_CHARACTER_CHEEKX_SCALE,
	SLIDER_CHARACTER_CHEEKY_SCALE,
	SLIDER_CHARACTER_CHEEKZ_SCALE,
	SLIDER_CHARACTER_CHINX_SCALE,
	SLIDER_CHARACTER_CHINY_SCALE,
	SLIDER_CHARACTER_CHINZ_SCALE,
	SLIDER_CHARACTER_CRANIUMX_SCALE,
	SLIDER_CHARACTER_CRANIUMY_SCALE,
	SLIDER_CHARACTER_CRANIUMZ_SCALE,
	SLIDER_CHARACTER_JAWX_SCALE,
	SLIDER_CHARACTER_JAWY_SCALE,
	SLIDER_CHARACTER_JAWZ_SCALE,
	SLIDER_CHARACTER_NOSEX_SCALE,
	SLIDER_CHARACTER_NOSEY_SCALE,
	SLIDER_CHARACTER_NOSEZ_SCALE,
	SLIDER_MAP_SCALE,
	SLIDER_BASE_RGB00,
	SLIDER_BASE_RGB01,
	SLIDER_BASE_RGB02,
	SLIDER_BASE_RGB10,
	SLIDER_BASE_RGB11,
	SLIDER_BASE_RGB12,
	SLIDER_BASE_RGB20,
	SLIDER_BASE_RGB21,
	SLIDER_BASE_RGB22,
	SLIDER_POWER_INFO_LEVEL,
	SLIDER_CUSTOM_CRITTER_LEVEL,
}SliderType;

float doSlider(			float lx, float ly, float lz, float wd, float xsc, float ysc, float value, int type, int color, int flags );
float doSliderTicks( float lx, float ly, float lz, float wd, float xsc, float ysc, float value, int type, int color, int flags, int count, float *tickValues);
float doVerticalSlider( float lx, float ly, float lz, float ht, float xsc, float ysc, float value, int type, int color );

float doSliderAll( float lx, float ly, float lz, float wd, float xsc, float ysc, float value, float minval, float maxval,
				  int type, int color, int flags, int count, float *tickValues);
void drawSliderBackground( float x, float y, float z, float wd, float sc, int flags );
#endif