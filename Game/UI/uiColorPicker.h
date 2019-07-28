#ifndef UICOLORPICKER_H
#define UICOLORPICKER_H

typedef struct ColorPicker
{
	int rgb_color;
	float hue;
	float saturation;
	float luminosity;

	int cur_hue_idx;
	int cur_sat_idx;
	int cur_lum_idx;
	int last_lum_idx;
	float trans_sc;
}ColorPicker;

typedef union Color Color;
void pickerSetfromColor( ColorPicker * picker, Color *color );

int drawColorPicker( ColorPicker * picker, float x, float y, float z, float wd, float ht, int ignoreInput);

int chatColorPickerWindow(void);
void startColorPickerWindow( char * pchChannel );
void pickerSetfromIntColor( ColorPicker * picker, int color );
#endif