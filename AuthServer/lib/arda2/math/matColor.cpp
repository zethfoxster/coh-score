/*****************************************************************************
	created:	2001/10/18
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matColor.h"
#include "arda2/math/matColorInt.h"

const matColor matColor::Black	(0.0f, 0.0f, 0.0f);
const matColor matColor::White	(1.0f, 1.0f, 1.0f);
const matColor matColor::Green	(0.0f, 1.0f, 0.0f);

const matVector4 Low(0.0f, 0.0f, 0.0f, 0.0f);
const matVector4 High(1.0f, 1.0f, 1.0f, 1.0f);

void matColor::Set( const matColorInt &color )
{
	static float inv255 = 1.0f / 255.0f;
	m_r = color.GetR() * inv255;
	m_g = color.GetG() * inv255;
	m_b = color.GetB() * inv255;
	m_a = color.GetA() * inv255;
}


void matColor::Set( const matVector4& colorvec )
{
	matVector4 temp(colorvec);
	temp.StoreComponentClamp(Low, High);
	m_r = colorvec.x;
	m_g = colorvec.y;
	m_b = colorvec.z;
	m_a = colorvec.w;
}

void matColor::Interpolate(	float fInterpolant, 
							const matColor& color0, 
							const matColor& color1 )
{
	matVector4	c0(color0.m_r, color0.m_g, color0.m_b, color0.m_a),
				c1(color1.m_r, color1.m_g, color1.m_b, color1.m_a);

	matVector4 newVec = matLerp(c0, c1, fInterpolant);
	Set(newVec);
}

void matColor::Interpolate(	float fInterpolant, 
							const matColor& color0, 
							const matColor& color1, 
							const matColor& color2, 
							const matColor& color3 )
{
	fInterpolant = matClamp(fInterpolant, 0.0f, 1.0f);
	
	matVector4	c0(color0.m_r, color0.m_g, color0.m_b, color0.m_a),
				c1(color1.m_r, color1.m_g, color1.m_b, color1.m_a),
				c2(color2.m_r, color2.m_g, color2.m_b, color2.m_a),
				c3(color3.m_r, color3.m_g, color3.m_b, color3.m_a), out;

	out.StoreCatmullRom(c0, c1, c2, c3, fInterpolant);
	Set(out);
}
