/*****************************************************************************
	created:	2002/03/26
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Ryan M. Prescott
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matVector4.h"

void matVector4::StoreComponentClamp(	const matVector4& min,
										const matVector4& max)
{
	x = matClamp(x, min.x, max.x);
	y = matClamp(y, min.y, max.y);
	z = matClamp(z, min.z, max.z);
	w = matClamp(w, min.w, max.w);
}

matVector4 ComponentClamp(	const matVector4& val, 
							const matVector4& min,
							const matVector4& max)
{
	// componentwise clamp
	matVector4 temp(val);
	temp.StoreComponentClamp(min, max);
	return temp;
}

void matVector4::Normalize()
{
	matBaseVec4Normalize(this, this);
}

float matVector4::Magnitude() const
{
	return (float)sqrt(x*x + y*y + z*z + w*w);
}

float matVector4::MagnitudeSquared() const
{
	return x*x + y*y + z*z + w*w;
}

float matVector4::DotProduct(const matVector4& v1) const
{
	return matBaseVec4Dot(&v1, this);
}

void  matVector4::StoreCrossProduct(const matVector4& v1, 
									const matVector4& v2,
									const matVector4& v3)
{
	matBaseVec4Cross(this, &v1, &v2, &v3);
}

void matVector4::StoreScale(const matVector4& vec, 
							float s)
{
	matBaseVec4Scale(this, &vec, s);
}

void matVector4::StoreTransform(const matVector4& vec, 
								const matMatrix4x4& mat)
{
	matBaseVec4Transform(this, &vec, (const matBaseMATRIX*)&mat);
}

void matVector4::Transform(const matMatrix4x4& mat)
{
	matBaseVec4Transform(this, this, (const matBaseMATRIX*)&mat);
}

