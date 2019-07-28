/*****************************************************************************
	created:	2001-11-12
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matPlane.h"

bool matPlane::IntersectsSegment( IN const matVector3 &vStart, IN const matVector3 &vEnd, OUT float *pT, OUT matVector3 *pIntersection )
{
	const float fStartT(GetSignedDistance(vStart));
	const float fEndT(GetSignedDistance(vEnd));
	
	// check to make sure the endpoints span the plane
	if ( fStartT * fEndT > 0.0f )
		return false;
	
	if ( pT || pIntersection )
	{
		float t;

		// check for start on the plane
		if ( fStartT == 0.0f )
		{
			t = 0.0f;
		}
		// check for end on the plane
		else if ( fEndT == 0.0f )
		{
			t = 1.0f;
		}
		else
		{
			// solve parametric equation to find the intersection point
			float absStartT = fabsf(fStartT);
			float absEndT   = fabsf(fEndT);
			t = absStartT / (absStartT + absEndT);
		}

		if ( pT )
			*pT = t;

		if ( pIntersection )
			*pIntersection = vStart + t * (vEnd - vStart);
	}
	
	return true;
}

bool matPlane::IntersectsRay( IN matVector3 &vStart, IN const matVector3 &vDir, OUT float *pT, OUT matVector3 *pIntersection )
{
	float cosAlpha = vDir.DotProduct( GetNormal() );

	// ray is parallel
	if ( cosAlpha == 0.0f )
		return false;

	float deltaD = GetSignedDistance(vStart);

	float t = -deltaD / cosAlpha;

	if ( t < 0.0f )
		return false;

	if ( pT )
		*pT = t;

	if ( pIntersection )
		*pIntersection = vStart + t * vDir;

	return true;
}

bool matPlane::IntersectsRayOneSided( IN matVector3 &vStart, IN const matVector3 &vDir, OUT float *pT, OUT matVector3 *pIntersection )
{
    float cosAlpha = vDir.DotProduct( GetNormal() );

    // ray is parallel or pointing away from the plane
    if ( cosAlpha >= 0.0f )
        return false;

    float deltaD = GetSignedDistance(vStart);

    float t = -deltaD / cosAlpha;

    if ( t < 0.0f )
        return false;

    if ( pT )
        *pT = t;

    if ( pIntersection )
        *pIntersection = vStart + t * vDir;

    return true;
}

