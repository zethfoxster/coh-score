/*****************************************************************************
	created:	2002/04/10
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Ryan Prescott

	purpose:	provide colorspace conversion
*****************************************************************************/

#ifndef __DEFINEDmatConvert
#define __DEFINEDmatConvert

class matVector3;

void matConvertXYZtoRGB(matVector3 &RGBvec, const matVector3 &XYZvec);


void matConvertRGBtoXYZ(matVector3 &XYZvec, const matVector3 &RGBvec);


void matConvertXYZtoLAB(matVector3 &LABvec, const matVector3 &XYZvec);


void matConvertRGBtoHSV(matVector3& HSVvec, const matVector3& RGBvec);


void matConvertHSVtoRGB(matVector3& RGBvec, const matVector3& HSVvec);


#endif//__DEFINEDmatConvert
