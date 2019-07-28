/*****************************************************************************
	created:	2001-11-02
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matPlane
#define   INCLUDED_matPlane
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matVector3.h"

class matPlane : public matBasePLANE
{
public:
	matPlane() {};
	matPlane(const matVector3 &vNormal, float fD)	: matBasePLANE(vNormal.x, vNormal.y, vNormal.z, fD) {;}
	matPlane(const matPlane &plane) : matBasePLANE(plane) {}
	matPlane(float fA, float fB, float fC, float fD) : matBasePLANE(fA, fB, fC, fD) {;}
	matPlane(const matVector3 &vPlaneNormal, const matVector3 &vPointOnPlane) { Set(vPlaneNormal, vPointOnPlane); }	
	matPlane(const matVector3 &vPoint0, const matVector3 &vPoint1, const matVector3 &vPoint2) { Set(vPoint0, vPoint1, vPoint2); }
	
	void Set(const matVector3 &vNormal, float fD)		{ a = vNormal.x; b = vNormal.y; c = vNormal.z; d = fD; }
	void Set(const matPlane &plane)						{ *this = plane; }
	void Set(float fA, float fB, float fC, float fD)	{ a = fA; b = fB; c = fC; d = fD; }
	void Set(const matVector3 &vNormal, const matVector3 &vPointOnPlane) { matBasePlaneFromPointNormal(this, &vPointOnPlane, &vNormal); }
	void Set(const matVector3 &vPoint0, const matVector3 &vPoint1, const matVector3 &vPoint2) { matBasePlaneFromPoints(this, &vPoint0, &vPoint1, &vPoint2); }
	
	const matVector3 &GetNormal() const		{ return *(const matVector3 *)&a; }
	float GetConstant() const				{ return d; }
	float A() const							{ return a; }
	float B() const							{ return b; }
	float C() const							{ return c; }
	float D() const							{ return d; }
	
	float GetSignedDistance(const matVector3 &vPoint ) const	{ return GetNormal().DotProduct(vPoint) + d; }
	float GetHeight(float fX, float fZ ) const	
	{
		if ( b != 0.0f )
			return -( a * fX + c * fZ + d ) / b; 
		else 
			return 0.0f;
	}
	
	bool IntersectsSegment( IN const matVector3 &vStart, IN const matVector3 &vEnd, OUT float *pT = NULL, OUT matVector3 *pIntersection = NULL );
    bool IntersectsRay( IN matVector3 &vStart, IN const matVector3 &vDir, OUT float *pT = NULL, OUT matVector3 *pIntersection = NULL );
    bool IntersectsRayOneSided( IN matVector3 &vStart, IN const matVector3 &vDir, OUT float *pT = NULL, OUT matVector3 *pIntersection = NULL );

    void Translate( const matVector3& offset )
    {
        d -= GetNormal().DotProduct(offset);
    }
	
private:
};

#endif // INCLUDED_matPlane
