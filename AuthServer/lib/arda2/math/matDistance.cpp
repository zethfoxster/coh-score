/*****************************************************************************
created:	2002/10/23
copyright:	2002, NCSoft. All Rights Reserved
author(s):	Jason Beardsley

purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matDistance.h"


/********************************************************************/

static float L2L(const matVector3& l0start, const matVector3& l0dir,
                 const matVector3& l1start, const matVector3& l1dir,
                 float& line0u, float& line1u)
{
    const matVector3 diff = l0start - l1start;

    float fA00 = l0dir.MagnitudeSquared();
    float fA01 = -DotProduct(l0dir, l1dir);
    float fA11 = l1dir.MagnitudeSquared();
    float fB0 = DotProduct(diff, l0dir);
    float fC = diff.MagnitudeSquared();
    float fDet = fabsf(fA00*fA11 - fA01*fA01);
    float fS, fT, fSqrDist;

    if (fDet >= 1.0e-05f)
    {
        // lines are not parallel
        float fB1 = -DotProduct(diff, l1dir);
        float fInvDet = 1.0f / fDet;
        fS = (fA01*fB1 - fA11*fB0) * fInvDet;
        fT = (fA01*fB0 - fA00*fB1) * fInvDet;
        fSqrDist = fS * (fA00*fS + fA01*fT + 2.0f*fB0) + fT * (fA01*fS + fA11*fT + 2.0f*fB1) + fC;
    }
    else
    {
        // lines are parallel, select any closest pair of points
        if ( fB0 < 0.0f )
        {
            fS = -fB0 / fA00;
            fT = 0.0f;
        }
        else
        {
            fS = 0.0f;
            fT = DotProduct(diff, l1dir) / fA11;
        }
        fSqrDist = fB0*fS + fC;
    }

    line0u = fS;
    line1u = fT;

    return fabsf(fSqrDist);
}

/********************************************************************/

/**
* Compute distance^2 between two (infinite) lines, and compute the
* points on those lines closest to the other line.
*
* @param line0p0, line0p1 - any two points on line0
* @param line1p0, line1p1 - any two points on line1
* @param line0pt [out] - point on line0 closest to line1
* @param line1pt [out] - point on line1 closest to line0
*
* @returns Squared distance between line0 and line1
**/

float matDistance::LineToLineSquared(const matVector3& line0p0, const matVector3& line0p1,
                                     const matVector3& line1p0, const matVector3& line1p1,
                                     matVector3* line0pt, matVector3* line1pt)
{
    float u0, u1;
    matVector3 line0dir = line0p1 - line0p0;
    matVector3 line1dir = line1p1 - line1p0;
    float ret = L2L(line0p0, line0dir, line1p0, line1dir, u0, u1);
    if (line0pt != NULL)
    {
        *line0pt = line0p0 + u0 * line0dir;
    }
    if (line1pt != NULL)
    {
        *line1pt = line1p0 + u1 * line1dir;
    }
    return ret;
}

/********************************************************************/

/**
* Compute distance^2 between two line segments, and compute the
* points on those segments closest to the other segment.
*
* @param seg0p0, seg0p1 - endpoints of segment0
* @param seg1p0, seg1p1 - endpoints of segment1
* @param seg0pt [out] - point on segment0 closest to segment1
* @param seg1pt [out] - point on segment1 closest to segment0
*
* @returns Squared distance between segment0 and segment1
**/

float matDistance::SegmentToSegmentSquared(const matVector3& seg0p0, const matVector3& seg0p1,
                                           const matVector3& seg1p0, const matVector3& seg1p1,
                                           matVector3* seg0pt, matVector3* seg1pt)
{
    float u0, u1;
    matVector3 seg0dir = seg0p1 - seg0p0;
    matVector3 seg1dir = seg1p1 - seg1p0;
    float ret = L2L(seg0p0, seg0dir, seg1p0, seg1dir, u0, u1);
    u0 = matClamp(u0, 0.0f, 1.0f);
    u1 = matClamp(u1, 0.0f, 1.0f);
    // recompute the distance, since it was between the infinite lines
    matVector3 seg0closest = seg0p0 + u0 * seg0dir;
    matVector3 seg1closest = seg1p0 + u1 * seg1dir;
    ret = seg0closest.DistanceBetweenSquared(seg1closest);
    if (seg0pt != NULL)
    {
        *seg0pt = seg0closest;
    }
    if (seg1pt != NULL)
    {
        *seg1pt = seg1closest;
    }
    return ret;
}

/********************************************************************/

/**
* Compute distance^2 between a point and (infinite) line, as well as
* the closest point on the line to the point.  Get the point?
*
* @param p - the point
* @param linep0, linep1 - any two points on the line
* @param pClosestPt [out] - closest point on line to p
*
* @returns Squared distance between line and p.
**/

float matDistance::PointToLineSquared(const matVector3& p,
                                      const matVector3& linep0, const matVector3& linep1,
                                      matVector3* pClosestPt)
{
    matVector3 diff = linep1 - linep0;

    // check for singularity
    float diffMag2 = diff.MagnitudeSquared();
    if (diffMag2 < 1.0e-04)
    {
        return linep0.DistanceBetweenSquared(p);
    }

    float u =
        (p.x - linep0.x) * diff.x +
        (p.y - linep0.y) * diff.y +
        (p.z - linep0.z) * diff.z;
    u /= diffMag2;

    matVector3 pInt;
    pInt.x = linep0.x + u * diff.x;
    pInt.y = linep0.y + u * diff.y;
    pInt.z = linep0.z + u * diff.z;

    if (pClosestPt)
    {
        *pClosestPt = pInt;
    }

    return pInt.DistanceBetweenSquared(p);
}

/********************************************************************/

/**
* Compute distance^2 between a point and line segment, as well as
* the closest point on the segment to the point.
*
* @param p - the point
* @param segp0, segp1 - segment endpoints
* @param pClosestPt [out] - closest point on segment to p
*
* @returns Squared distance between segment and p.
**/

float matDistance::PointToSegmentSquared(const matVector3& p,
                                         const matVector3& segp0, const matVector3& segp1,
                                         matVector3* pClosestPt)
{
    matVector3 diff = segp1 - segp0;

    // check for singularity
    float diffMag2 = diff.MagnitudeSquared();
    if (diffMag2 < 1.0e-04)
    {
        return segp0.DistanceBetweenSquared(p);
    }

    float u =
        (p.x - segp0.x) * diff.x +
        (p.y - segp0.y) * diff.y +
        (p.z - segp0.z) * diff.z;
    u /= diffMag2;
    u = matClamp(u, 0.0f, 1.0f);

    matVector3 pInt;
    pInt.x = segp0.x + u * diff.x;
    pInt.y = segp0.y + u * diff.y;
    pInt.z = segp0.z + u * diff.z;

    if (pClosestPt)
    {
        *pClosestPt = pInt;
    }

    return pInt.DistanceBetweenSquared(p);
}

/********************************************************************/

/**
* Compute distance^2 between a point and an aligned box.
*
* @param p - the point
* @param bMin, bMax - min/max points of box
*
* @returns Squared distance between box and p.  If p is inside box,
* this is zero.
**/

float matDistance::PointToAlignedBoxSquared(const matVector3& p,
                                            const matVector3& bMin, const matVector3& bMax)
{
    float dist2 = 0.0f;

    if (p.x < bMin.x)
    {
        dist2 += matSquare(p.x - bMin.x);
    }
    else if (p.x > bMax.x)
    {
        dist2 += matSquare(p.x - bMax.x);
    }

    if (p.y < bMin.y)
    {
        dist2 += matSquare(p.y - bMin.y);
    }
    else if (p.y > bMax.y)
    {
        dist2 += matSquare(p.y - bMax.y);
    }

    if (p.z < bMin.z)
    {
        dist2 += matSquare(p.z - bMin.z);
    }
    else if (p.z > bMax.z)
    {
        dist2 += matSquare(p.z - bMax.z);
    }

    return dist2;
}

/********************************************************************/

/**
* Compute distance^2 between an infinite line and an aligned box, as
* well as point on line closest to box.
*
* @param linep0, linep1 - any two points on line
* @param bMin, bMax - min/max points of box
* @param pClosestPt [out] - point on line closest to box
*
* @returns Squared distance between box and line.
**/

float matDistance::LineToAlignedBoxSquared(const matVector3& /*linep0*/, const matVector3& /*linep1*/,
                                           const matVector3& /*bMin*/, const matVector3& /*bMax*/,
                                           matVector3* /*pClosestPt*/)
{
    // TODO: implement
    ERR_UNIMPLEMENTED();
    return 0.0f;
}

/********************************************************************/

/**
* Compute distance^2 between a line segment and an aligned box, as
* well as point on segment closest to box.
*
* @param segp0, segp1 - segment endpoints
* @param bMin, bMax - min/max points of box
* @param pClosestPt [out] - point on segment closest to box
*
* @returns Squared distance between segment and line.
**/

float matDistance::SegmentToAlignedBoxSquared(const matVector3& /*segp0*/, const matVector3& /*segp1*/,
                                              const matVector3& /*bMin*/, const matVector3& /*bMax*/,
                                              matVector3* /*pClosestPt*/)
{
    // TODO: implement
    ERR_UNIMPLEMENTED();
    return 0.0f;
}


/**
* Approximates distance^2 between a line segment and an aligned
* box, as well as the approximate point on the segment closest
* to the box. Uses an iterative, interpolative approach to get
* close enough to an answer without doing all that yucky math.
*
* @param segp0, segp1 - segment endpoints
* @param bMin, bMax - min/max points of box
* @param iMaxIter - max number of iterations to try
* @param pClosestPt [out] - point on segment closest to box; 
*                           keep as NULL if no result desired
*
* @returns Squared distance between segment and line.
**/

float matDistance::SegmentToAlignedBoxSquaredApprox(const matVector3& segp0, const matVector3& segp1,
                                                    const matVector3& bMin, const matVector3& bMax,
                                                    int iMaxIter, matVector3* pClosestPt)
{
    matVector3 P0 = segp0;
    float fD20 = PointToAlignedBoxSquared(P0, bMin, bMax);
    // early out
    if (fD20 <= VEC3_COMPARE_EPSILON)
    {
        if (pClosestPt)
        {
            *pClosestPt = P0;
        }
        return fD20;
    }

    matVector3 P1 = segp1;
    float fD21 = PointToAlignedBoxSquared(P1, bMin, bMax);
    // early out
    if (fD21 <= VEC3_COMPARE_EPSILON)
    {
        if (pClosestPt)
        {
            *pClosestPt = P1;
        }
        return fD21;
    }

    matVector3 PMid;
    float fD2Mid = 0.0f;

    // interpolate
    int ix;
    for (ix = 0; ix < iMaxIter; ix++)
    {
        // derive the midpoint
        PMid = P1 - P0;
        PMid /= 2;
        PMid += P0;

        // distance from mid to box
        fD2Mid = PointToAlignedBoxSquared(PMid, bMin, bMax);
        // early out
        if (fD2Mid <= VEC3_COMPARE_EPSILON)
        {
            if (pClosestPt)
            {
                *pClosestPt = PMid;
            }
            return fD2Mid;
        }

        // compare the relative distances to see which way to go
        float fDelta0 = fabsf(fD20 - fD2Mid);
        float fDelta1 = fabsf(fD21 - fD2Mid);
        float fSmDelta = 0.0f;  // smallest delta
        if (fDelta0 < fDelta1)
        {
            fSmDelta = fDelta0;
            // interpolate between P0 and PMid
            P1 = PMid;
            fD21 = fD2Mid;
        }
        else
        {
            fSmDelta = fDelta1;
            // interpolate between PMid and P1
            P0 = PMid;
            fD20 = fD2Mid;
        }

        if (fSmDelta <= 0.01f) 
        {
            // the difference in the distances from the two points to the 
            // box is small enough as to be negligible.
            break;
        }
    }

    // our good enough distance is the distance from our last midpoint
    if (pClosestPt)
    {
        *pClosestPt = PMid;
    }
    return fD2Mid;
}
