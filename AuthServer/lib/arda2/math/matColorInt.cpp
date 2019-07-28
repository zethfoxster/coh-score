/*****************************************************************************
	created:	2001/10/18
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matColorInt.h"
#include "arda2/math/matColor.h"
#include "arda2/math/matVector4.h"

const matColorInt matColorInt::Black   (  0,   0,   0);
const matColorInt matColorInt::Blue    (  0,   0, 255);
const matColorInt matColorInt::Cyan    (  0, 255, 255);
const matColorInt matColorInt::Green   (  0, 255,   0);
const matColorInt matColorInt::Grey    (128, 128, 128);
const matColorInt matColorInt::Magenta (255,   0, 255);
const matColorInt matColorInt::Red     (255,   0,   0);
const matColorInt matColorInt::White   (255, 255, 255);
const matColorInt matColorInt::Yellow  (255, 255,   0);

// component wise min and max
const matVector4 Low(0.0f, 0.0f, 0.0f, 0.0f);
const matVector4 High(1.0f, 1.0f, 1.0f, 1.0f);


// internal method for setting colors from float values; used for conversion and interpolation
void matColorInt::SetFloat( float r, float g, float b, float a )
{
	m_r = static_cast<uint8>(matClamp(matFloatToIntRound(r), 0, 255));
	m_g = static_cast<uint8>(matClamp(matFloatToIntRound(g), 0, 255));
	m_b = static_cast<uint8>(matClamp(matFloatToIntRound(b), 0, 255));
	m_a = static_cast<uint8>(matClamp(matFloatToIntRound(a), 0, 255));
}

void matColorInt::Set( const matColor &color )
{
	SetFloat(color.GetR() * 255.0f, color.GetG() * 255.0f, color.GetB() * 255.0f, color.GetA() * 255.0f);
}

void matColorInt::Set( const matVector4& vec )
{
	SetFloat(vec.x * 255.0f, vec.y * 255.0f, vec.z * 255.0f, vec.w * 255.0f);
}

matVector4 matColorInt::GetVec() const
{
	matVector4 temp;
	temp.Set(m_r, m_g, m_b, m_a);
	temp *= 1.0f/255.0f;
	return temp;
}

void matColorInt::Interpolate(float fInterpolant, 
							  const matColorInt color0, 
							  const matColorInt color1 )
{
	fInterpolant = matClamp(fInterpolant, 0.0f, 1.0f);
	float r = color0.m_r + fInterpolant * (color1.m_r - color0.m_r);
	float g = color0.m_g + fInterpolant * (color1.m_g - color0.m_g);
	float b = color0.m_b + fInterpolant * (color1.m_b - color0.m_b);
	float a = color0.m_a + fInterpolant * (color1.m_a - color0.m_a);
	SetFloat(r,g,b,a);
}

void matColorInt::Interpolate(	float fInterpolant, 
								const matColorInt color0, 
								const matColorInt color1, 
								const matColorInt color2, 
								const matColorInt color3 )
{
	fInterpolant = matClamp(fInterpolant, 0.0f, 1.0f);
	
	matVector4	c0(color0.m_r, color0.m_g, color0.m_b, color0.m_a),
				c1(color1.m_r, color1.m_g, color1.m_b, color1.m_a),
				c2(color2.m_r, color2.m_g, color2.m_b, color2.m_a),
				c3(color3.m_r, color3.m_g, color3.m_b, color3.m_a), out;

	out.StoreCatmullRom(c0, c1, c2, c3, fInterpolant);
	SetFloat(out.x, out.y, out.z, out.w);
}
