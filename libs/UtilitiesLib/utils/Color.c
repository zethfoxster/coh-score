#include "Color.h"
#include "mathutil.h"

Color ColorWhite = {0xFF, 0xFF, 0xFF, 0xFF};
Color ColorBlack = {0, 0, 0, 0xFF};
Color ColorRed = {0xFF, 0, 0, 0xFF};
Color ColorGreen = {0, 0xFF, 0, 0xFF};
Color ColorBlue = {0, 0, 0xFF, 0xFF};
Color ColorLightRed = {0xFF, 0x7F, 0x7F, 0xFF};
Color ColorOrange = {0xFF, 0x80, 0x40, 0xFF};
Color ColorNull = {0, 0, 0, 0};

ColorPair colorPairNone = {{ 0 } , { 0 }};

Color colorFlip( Color c )
{
	Color col;

	col.r = c.a;
	col.g = c.b;
	col.b = c.g;
	col.a = c.r;

	return col;
}

int colorEqualsIgnoreAlpha( Color c1, Color c2 )
{
	if( c1.r == c2.r &&
		c1.g == c2.g &&
		c1.b == c2.b )
		return 1;

	return 0;
}


U32 darkenColorRGBA(U32 rgba, int amount)
{
	Color c;
	setColorFromRGBA(&c, rgba);
	c.r = MAX(c.r-amount, 0);
	c.g = MAX(c.g-amount, 0);
	c.b = MAX(c.b-amount, 0);
	return RGBAFromColor(c);
}

U32 lightenColorRGBA(U32 rgba, int amount)
{
	Color c;
	setColorFromRGBA(&c, rgba);
	c.r = MAX(c.r+amount, 0);
	c.g = MAX(c.g+amount, 0);
	c.b = MAX(c.b+amount, 0);
	return RGBAFromColor(c);
}


static INLINEDBG U8 F32ToColorComponent(F32 f)
{
	int ret = round(f * 255.f);
	return ret<0?0:ret>255?255:ret;
}

void vec3ToColor(Color *clr, const Vec3 vec)
{
	clr->r = F32ToColorComponent(vec[0]);
	clr->g = F32ToColorComponent(vec[1]);
	clr->b = F32ToColorComponent(vec[2]);
	clr->a = 255;
}

void vec4ToColor(Color *clr, const Vec4 vec)
{
	clr->r = F32ToColorComponent(vec[0]);
	clr->g = F32ToColorComponent(vec[1]);
	clr->b = F32ToColorComponent(vec[2]);
	clr->a = F32ToColorComponent(vec[3]);
}
