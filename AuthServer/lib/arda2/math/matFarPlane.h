/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    Plane equation with far space support
 *  modified:   $DateTime: 2005/11/17 10:07:52 $
 *              $Author: cthurow $
 *              $Change: 178508 $
 *              @file
 */

#ifndef   INCLUDED_matFarPlane
#define   INCLUDED_matFarPlane
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matVector3.h"
#include "arda2/math/matFarPosition.h"
#include "arda2/math/matPlane.h"
#include "arda2/core/corStlVector.h"

class matFarPlane
{
public:
	matFarPlane() {};
	matFarPlane(const matVector3 &vPlaneNormal, const matFarPosition &vPointOnPlane) { Set(vPlaneNormal, vPointOnPlane); }	
	matFarPlane(const matFarPosition &vPoint0, const matFarPosition &vPoint1, const matFarPosition &vPoint2) { Set(vPoint0, vPoint1, vPoint2); }
	
	void Set( const matVector3 &vNormal, const matFarPosition &vPointOnPlane );
	void Set( const matFarPosition &vPoint0, const matFarPosition &vPoint1, const matFarPosition &vPoint2 );
	const matVector3 &GetNormal() const		{ return m_localPlane.GetNormal(); }
	
	float GetSignedDistance(const matFarPosition &vPoint ) const
    {
        return m_localPlane.GetSignedDistance(vPoint.GetRelativeVector(m_segment));
    }

    void Translate( const matVector3& offset );

    bool IntersectsSegment( IN const matFarPosition &vStart, IN const matFarPosition &vEnd, OUT float *pT = NULL, OUT matFarPosition *pIntersection = NULL );
    bool IntersectsRay( IN matFarPosition &vStart, IN const matVector3 &vDir, OUT float *pT = NULL, OUT matFarPosition *pIntersection = NULL );
    bool IntersectsRayOneSided( IN matFarPosition &vStart, IN const matVector3 &vDir, OUT float *pT = NULL, OUT matFarPosition *pIntersection = NULL );
	
private:
    matFarSegment   m_segment;
    matPlane        m_localPlane;
};


#endif // INCLUDED_matFarPlane
