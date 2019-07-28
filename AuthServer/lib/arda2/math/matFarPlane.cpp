/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    Plane equation with far space support
 *  modified:   $DateTime: 2005/11/17 10:07:52 $
 *              $Author: cthurow $
 *              $Change: 178508 $
 *              @file
 */

#include "arda2/math/matFirst.h"
#include "arda2/math/matFarPlane.h"


void matFarPlane::Set( const matVector3 &vNormal, const matFarPosition &vPointOnPlane )
{
    m_segment = vPointOnPlane.GetSegment();
    m_localPlane.Set(vNormal, vPointOnPlane.GetOffset());
}


void matFarPlane::Set( const matFarPosition &vPoint0, const matFarPosition &vPoint1, const matFarPosition &vPoint2 )
{
    // find center of points
    matFarPosition center = vPoint0;
    matVector3 offset = 0.5f * ((vPoint1 - vPoint0) + (vPoint2 - vPoint0));
    center.Translate(offset);
    m_segment = center.GetSegment();
    m_localPlane.Set(vPoint0.GetRelativeVector(m_segment), vPoint1.GetRelativeVector(m_segment), vPoint2.GetRelativeVector(m_segment));
}


void matFarPlane::Translate( const matVector3& offset )
{
    m_localPlane.d -= m_localPlane.GetNormal().DotProduct(offset);
}


bool matFarPlane::IntersectsSegment( IN const matFarPosition &vStart, IN const matFarPosition &vEnd, OUT float *pT, OUT matFarPosition *pIntersection )

{
    matVector3 vLocalStart = vStart.GetRelativeVector(m_segment);
    matVector3 vLocalEnd = vEnd.GetRelativeVector(m_segment);
	const float fStartT = m_localPlane.GetSignedDistance(vLocalStart);
	const float fEndT = m_localPlane.GetSignedDistance(vLocalEnd);
	
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
        {
            *pIntersection = vStart;
            pIntersection->Translate(t * (vEnd - vStart));
        }
	}
	
	return true;
}

bool matFarPlane::IntersectsRay( IN matFarPosition &vStart, IN const matVector3 &vDir, OUT float *pT, OUT matFarPosition *pIntersection)
{
    float cosAlpha = vDir.DotProduct( GetNormal() );

    // ray is parallel
    if ( cosAlpha == 0.0f )
        return false;

    matVector3 vLocalStart = vStart.GetRelativeVector(m_segment);
    float deltaD = m_localPlane.GetSignedDistance(vLocalStart);

    float t = -deltaD / cosAlpha;
	if ( t < 0.0f )
		return false;

    if ( pT )
        *pT = t;

    if ( pIntersection )
    {
        *pIntersection = vStart;
        pIntersection->Translate(t * vDir);
    }

    return true;
}

bool matFarPlane::IntersectsRayOneSided( IN matFarPosition &vStart, IN const matVector3 &vDir, OUT float *pT, OUT matFarPosition *pIntersection)
{
    float cosAlpha = vDir.DotProduct( GetNormal() );

    // ray is parallel or pointing away from the plane
    if ( cosAlpha >= 0.0f )
        return false;

    matVector3 vLocalStart = vStart.GetRelativeVector(m_segment);
    float deltaD = m_localPlane.GetSignedDistance(vLocalStart);

    float t = -deltaD / cosAlpha;
    if ( t < 0.0f )
        return false;

    if ( pT )
        *pT = t;

    if ( pIntersection )
    {
        *pIntersection = vStart;
        pIntersection->Translate(t * vDir);
    }

    return true;
}

