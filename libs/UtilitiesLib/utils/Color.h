/* File Color.h
 *	Defines a structure for holding RGBA color information that can be accessed
 *	in 3 different formats.
 *
 *
 */

#ifndef COLOR_H
#define COLOR_H

#include "stdtypes.h"

// in memory: r g b a
typedef union Color
{
	// Allows direct access to individual components.
	struct{
		U8 r;
		U8 g;
		U8 b;
		U8 a;
	};

	// Allows the color to be moved around like an integer.
	U32 integer;

	// Allows color component indexing like an array.
	U8 rgba[4];

	U8 rgb[3];

} Color;

typedef union Color565
{
	// Allows the color to be moved around like an integer.
	U16 integer;

	// Allows direct access to individual components.
	struct{
		U16 b:5;
		U16 g:6;
		U16 r:5;
	};
} Color565;


#define COLOR_CAST(integer)	*(Color*)&integer

// Color constructors
static INLINEDBG Color CreateColor(U8 r, U8 g, U8 b, U8 a)
{
	Color c; 
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return c;
}

static INLINEDBG Color colorFromU8(U8 *rgba)
{
	Color c; 
	c.r = rgba[0];
	c.g = rgba[1];
	c.b = rgba[2];
	c.a = rgba[3];
	return c;
}

static INLINEDBG void setColorFromARGB(Color *c, int argb)
{
	c->b = argb & 0xff;
	c->g = (argb >> 8) & 0xff;
	c->r = (argb >> 16) & 0xff;
	c->a = (argb >> 24) & 0xff;
}

static INLINEDBG void setColorFromABGR(Color *c, int abgr)
{
	c->r = abgr & 0xff;
	c->g = (abgr >> 8) & 0xff;
	c->b = (abgr >> 16) & 0xff;
	c->a = (abgr >> 24) & 0xff;
}

static INLINEDBG U32 ARGBFromColor(Color c)
{
	return (c.a << 24) | (c.r << 16) | (c.g << 8) | (c.b);
}

static INLINEDBG Color ARGBToColor(int argb)
{
	Color c;
	setColorFromARGB(&c, argb);
	return c;
}

static INLINEDBG U32 RGBAFromColor(Color c)
{
	return (c.r << 24) | (c.g << 16) | (c.b << 8) | (c.a);
}

static INLINEDBG void setColorFromRGBA(Color *c, int rgba)
{
	c->a = rgba & 0xff;
	c->b = (rgba >> 8) & 0xff;
	c->g = (rgba >> 16) & 0xff;
	c->r = (rgba >> 24) & 0xff;
}

static INLINEDBG Color CreateColorRGB(U8 r, U8 g, U8 b)
{
	Color c; 
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 255;
	return c;
}

Color colorFlip( Color c );
int colorEqualsIgnoreAlpha( Color c1, Color c2 );

static INLINEDBG F32 colorComponentToF32(U8 c)
{
	static const F32 U8TOF32_COLOR = 1.f / 255.f;
	return c * U8TOF32_COLOR;
}


static INLINEDBG void colorToVec4(Vec4 vec, const Color clr)
{
	vec[0] = colorComponentToF32(clr.r);
	vec[1] = colorComponentToF32(clr.g);
	vec[2] = colorComponentToF32(clr.b);
	vec[3] = colorComponentToF32(clr.a);
}

static INLINEDBG void u8ColorToVec4(Vec4 vec, const U8 *clr)
{
	vec[0] = colorComponentToF32(clr[0]);
	vec[1] = colorComponentToF32(clr[1]);
	vec[2] = colorComponentToF32(clr[2]);
	vec[3] = colorComponentToF32(clr[3]);
}

static INLINEDBG void colorToVec3(Vec3 vec, const Color clr)
{
	vec[0] = colorComponentToF32(clr.r);
	vec[1] = colorComponentToF32(clr.g);
	vec[2] = colorComponentToF32(clr.b);
}

static INLINEDBG void u8ColorToVec3(Vec3 vec, const U8 *clr)
{
	vec[0] = colorComponentToF32(clr[0]);
	vec[1] = colorComponentToF32(clr[1]);
	vec[2] = colorComponentToF32(clr[2]);
}

void vec4ToColor(Color *clr, const Vec4 vec);
void vec3ToColor(Color *clr, const Vec3 vec);

U32 darkenColorRGBA(U32 rgba, int amount);
U32 lightenColorRGBA(U32 rgba, int amount);


typedef struct ColorPair
{
	Color primary;
	Color secondary;
} ColorPair;

extern ColorPair colorPairNone;


extern Color ColorRed;
extern Color ColorBlue;
extern Color ColorGreen;
extern Color ColorWhite;
extern Color ColorBlack;
extern Color ColorLightRed;
extern Color ColorOrange;
extern Color ColorNull;

#endif