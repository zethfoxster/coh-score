/*****************************************************************************
	created:	2002/06/10
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matIntersect.h"

bool matIntersectTriangle(	const matVector3& vStart, const matVector3& vDir, 
							const matVector3& v0, const matVector3& v1, const matVector3& v2,
							float *pDist, float *pBaryU, float *pBaryV )
{
	// Find vectors for two edges sharing vert0
	matVector3 edge1 = v1 - v0;
	matVector3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate barycentric U parameter
	matVector3 pvec;
	pvec.StoreCrossProduct(vDir, edge2);

	// If determinant is near zero, ray lies in plane of triangle
	float fDet = edge1.DotProduct(pvec);

	matVector3 tvec;
	if( fDet > 0 )
	{
		tvec = vStart - v0;
	}
	else
	{
        // ignore back facing triangles
        return false;

		tvec = v0 - vStart;
		fDet = -fDet;
	}

	if( fDet < 0.0001f )
		return false;

	// Calculate barycentric U parameter and test bounds
	float fBaryU = tvec.DotProduct(pvec);
	if( fBaryU < 0.0f || fBaryU > fDet )
		return false;

	// Prepare to test barycentric V parameter
	matVector3 qvec;
	qvec.StoreCrossProduct(tvec, edge1);

	// Calculate barycentric V parameter and test bounds
	float fBaryV = vDir.DotProduct(qvec);
	if( fBaryV < 0.0f || fBaryU + fBaryV > fDet )
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	float fDist = edge2.DotProduct(qvec);
	float fInvDet = 1.0f / fDet;
	if ( pDist )
		*pDist = fDist * fInvDet;

	if ( pBaryU && pBaryV )
	{
		*pBaryU = fBaryU * fInvDet;
		*pBaryV = fBaryV * fInvDet;
	}

	return true;
}
