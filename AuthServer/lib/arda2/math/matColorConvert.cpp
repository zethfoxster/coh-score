
#include "arda2/math/matFirst.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matMatrix4x4.h"
#include "arda2/math/matColorConvert.h"

namespace ColorConvertPrivate
{
	// noone should be using values in here, except these functions
	const matMatrix4x4 XYZtoRGBmat(	3.240479f,-1.53715f ,-0.498535f, 0.0f,
									-0.969256f, 1.875991f, 0.041556f, 0.0f, 
									0.055648f,-0.204043f, 1.057311f, 0.0f, 
									0.0f, 0.0f, 0.0f, 0.0f);
	const matMatrix4x4 RGBtoXYZmat = Invert(XYZtoRGBmat);

	const matVector3 ReferenceWhiteRGB(1.0f, 1.0f, 1.0f);
	matVector3 ReferenceWhiteXYZ(matZero);

	// r,g,b values are from 0 to 1
	// h = [0,360], s = [0,1], v = [0,1]
	//		if s == 0, then h = -1 (undefined)
	inline void RGBtoHSV( float r, float g, float b, float *h, float *s, float *v )
	{
		float min, max, delta;
		min = r<g?r:g;
		min = min<b?min:b;
		max = r>g?r:g;
		max = max<b?max:b;
		*v = max;				// v
		delta = max - min;
		if( max != 0 )
			*s = delta / max;		// s
		else {
			// r = g = b = 0		// s = 0, v is undefined
			*s = 0;
			*h = -1;
			return;
		}
		if( r == max )
			*h = ( g - b ) / delta;		// between yellow & magenta
		else if( g == max )
			*h = 2 + ( b - r ) / delta;	// between cyan & yellow
		else
			*h = 4 + ( r - g ) / delta;	// between magenta & cyan
		*h *= 60;				// degrees
		if( *h < 0 )
			*h += 360;
		*h /= 10.0f;
	}
	inline void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
	{
		int i;
		float f, p, q, t;
		if( s == 0 ) {
			// achromatic (grey)
			*r = *g = *b = v;
			return;
		}
		h *= 6.0f;
		// h /= 60;			// sector 0 to 5
		i = (int)floorf( h );
		f = h - i;			// factorial part of h
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );
		switch( i ) {
			case 0:
				*r = v;
				*g = t;
				*b = p;
				break;
			case 1:
				*r = q;
				*g = v;
				*b = p;
				break;
			case 2:
				*r = p;
				*g = v;
				*b = t;
				break;
			case 3:
				*r = p;
				*g = q;
				*b = v;
				break;
			case 4:
				*r = t;
				*g = p;
				*b = v;
				break;
			default:		// case 5:
				*r = v;
				*g = p;
				*b = q;
				break;
		}
	}
}

void matConvertXYZtoRGB(matVector3 &RGBvec, const matVector3 &XYZvec)
{
	RGBvec = ColorConvertPrivate::XYZtoRGBmat.TransformPoint(XYZvec);
}

void matConvertRGBtoXYZ(matVector3 &XYZvec, const matVector3 &RGBvec)
{
	XYZvec = ColorConvertPrivate::RGBtoXYZmat.TransformPoint(RGBvec);
}

void matConvertXYZtoLAB(matVector3 &LABvec, const matVector3 &XYZvec)
{	
	using namespace ColorConvertPrivate;
	static float unprime, vnprime;

	if(ReferenceWhiteXYZ.IsZero())
	{
		matConvertRGBtoXYZ(ReferenceWhiteXYZ, ReferenceWhiteRGB);
		unprime = 4 * ReferenceWhiteXYZ.x / (ReferenceWhiteXYZ.x + 15 * ReferenceWhiteXYZ.y + 3 * ReferenceWhiteXYZ.z);	
		vnprime = 9 * ReferenceWhiteXYZ.y / (ReferenceWhiteXYZ.x + 15 * ReferenceWhiteXYZ.y + 3 * ReferenceWhiteXYZ.z); 
	}

	//Computation of CIE L*u*v* involves intermediate u' and v ' quantities,
	//where the prime denotes the successor to the obsolete 1960 CIE u and v
	//system:
//	float denom = 1.0f / (XYZvec.x + 15.0f * XYZvec.y + 3.0f * XYZvec.z);
//	float uprime = 4.0f * XYZvec.x * denom;	
//	float vprime = 9.0f * XYZvec.y * denom;

	float Lstar = -16 + 116 * powf(XYZvec.y / ReferenceWhiteXYZ.y, 1.0f / 3.0f); // Yn = 1.0f
	LABvec.x = Lstar;

	//Then compute u' and v ' - and L* as discussed earlier - for your colors.
	//Finally, compute:

	//ustar = 13 * Lstar * (uprime - unprime);
	//vstar = 13 * Lstar * (vprime - vnprime);

	//L*a*b* is computed as follows, for (X/Xn, Y/Yn, Z/Zn) > 0.01:

	LABvec.y = 500.0f * (powf(XYZvec.x / ReferenceWhiteXYZ.x, 1.0f/3.0f) - powf(XYZvec.y / ReferenceWhiteXYZ.y, 1.0f/3.0f));
	LABvec.z = 200.0f * (powf(XYZvec.y / ReferenceWhiteXYZ.y, 1.0f/3.0f) - powf(XYZvec.z / ReferenceWhiteXYZ.z, 1.0f/3.0f));
}

void matConvertRGBtoHSV(matVector3& HSVvec, const matVector3& RGBvec)
{
	ColorConvertPrivate::RGBtoHSV(RGBvec.x, RGBvec.y, RGBvec.z,&HSVvec.x, &HSVvec.y, &HSVvec.z);
}

void matConvertHSVtoRGB(matVector3& RGBvec, const matVector3& HSVvec)
{
	ColorConvertPrivate::HSVtoRGB(&RGBvec.x, &RGBvec.y, &RGBvec.z,HSVvec.x, HSVvec.y, HSVvec.z);
}
