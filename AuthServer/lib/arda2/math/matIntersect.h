/*****************************************************************************
    created:    2002/06/10
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matIntersect_h
#define   INCLUDED_matIntersect_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTransform.h"

//****************************************************************************
//* matIntersectTriangle()
//*
//* Given a ray origin (orig) and direction (dir), and three vertices of
//* of a triangle, this function returns true and the interpolated texture
//* coordinates if the ray intersects the triangle. Callers should pass and 
//* check the pDist value if they want to determine where on the ray the
//* intersection occured. A negative distance indicates the intersection
//* occurs before the origin.
//****************************************************************************
bool matIntersectTriangle(const matVector3& orig, const matVector3& dir, 
                          const matVector3& v0, const matVector3& v1, const matVector3& v2,
                          float *pDist = NULL, float *pBaryU = NULL, float *pBaryV = NULL );

inline bool matPointInSphere(const matVector3& pt, const matVector3& center, float radius)
{
    matVector3 diff = pt - center;
    return diff.MagnitudeSquared() < matSquare(radius);
}


inline bool matSpheresIntersect(const matVector3& s0Center, float s0Radius,
                             const matVector3& s1Center, float s1Radius)
{
    // return true if given spheres intersect
    matVector3 diff = s0Center - s1Center;
    return diff.MagnitudeSquared() < matSquare(s0Radius + s1Radius);
}


inline bool matSpheresIntersect2(const matVector3& s0Center, float s0Radius2,
                              const matVector3& s1Center, float s1Radius2)
{
    // return true if given spheres intersect, given squared radii
    // avoid 1 sqrt: (a+b)^2 = a^2 + 2ab + b^2; ab = sqrt(a^2*b^2)
    float fR2 = s0Radius2 + s1Radius2 + (2 * sqrtf(s0Radius2 * s1Radius2));
    matVector3 diff = s0Center - s1Center;
    return diff.MagnitudeSquared() < fR2;
}


inline bool matBoxesOverlap(const matVector3& b0Center, const matVector3& b0Extents,
                         const matMatrix4x4& b01Mat, const matVector3& b1Center, 
                         const matVector3& b1Extents)
{
    // return true if given boxes overlap (where box0 is 'aligned' and
    // box1 can be arbitrarily oriented)
    //
    // note: does not assume that 'extents' are non-negative
    // (i.e. computes min/max coordinates locally)
    // Since we are doing all tests in box 0's reference frame, then we can use
    // its untransformed center and extents to find their projected "radii"
    float b0MinX = b0Center.x - fabsf(b0Extents.x);
    float b0MaxX = b0Center.x + fabsf(b0Extents.x);

    // To calculate the projected radii for box 1, we must transform its center
    // into box 0 space, and then use the given similarity transform to project
    // its extents, given in relation to box 1's basis vectors, onto box 0's 
    // basis vectors.
    matVector3 b1CenterIn0 = b1Center;
    b1CenterIn0.TransformPoint(b01Mat);

    // To calculate box 1's extents projected onto box 0's x axis, we simply 
    // transform box 1's basis vectors, scaled by box 1's extents along them, into
    // box 0's reference frame, and then sum up the absolute value of their X 
    // components. This operation has been simplified by multiplying each of the
    // extents by the appropriate cell from the similarity transform, finding the
    // absolute value, and then summing them up.
    float b1RadiiX = fabsf(b1Extents.x * b01Mat._11) + 
        fabsf(b1Extents.y * b01Mat._21) + 
        fabsf(b1Extents.z * b01Mat._31);

    float b1MinX = b1CenterIn0.x - b1RadiiX;
    float b1MaxX = b1CenterIn0.x + b1RadiiX;

    bool overlapX =
        ((b0MinX <= b1MinX) && (b1MinX <= b0MaxX)) ||
        ((b0MinX <= b1MaxX) && (b1MaxX <= b0MaxX)) ||
        ((b1MinX <= b0MinX) && (b0MinX <= b1MaxX)) ||
        ((b1MinX <= b0MaxX) && (b0MaxX <= b1MaxX));

    //printf("BoxesOverlap: b0x=[%g,%g] b1x=[%g,%g] overlap=%d\n", b0MinX, b0MaxX, b1MinX, b1MaxX, overlapX);

    if (!overlapX)
    {
        return false;
    }

    float b0MinY = b0Center.y - fabsf(b0Extents.y);
    float b0MaxY = b0Center.y + fabsf(b0Extents.y);

    float b1RadiiY = fabsf(b1Extents.x * b01Mat._12) + 
        fabsf(b1Extents.y * b01Mat._22) + 
        fabsf(b1Extents.z * b01Mat._32);

    float b1MinY = b1CenterIn0.y - b1RadiiY;
    float b1MaxY = b1CenterIn0.y + b1RadiiY;

    bool overlapY =
        ((b0MinY <= b1MinY) && (b1MinY <= b0MaxY)) ||
        ((b0MinY <= b1MaxY) && (b1MaxY <= b0MaxY)) ||
        ((b1MinY <= b0MinY) && (b0MinY <= b1MaxY)) ||
        ((b1MinY <= b0MaxY) && (b0MaxY <= b1MaxY));

    //printf("BoxesOverlap: b0y=[%g,%g] b1y=[%g,%g] overlap=%d\n", b0MinY, b0MaxY, b1MinY, b1MaxY, overlapY);

    if (!overlapY)
    {
        return false;
    }

    float b0MinZ = b0Center.z - fabsf(b0Extents.z);
    float b0MaxZ = b0Center.z + fabsf(b0Extents.z);

    float b1RadiiZ = fabsf(b1Extents.x * b01Mat._13) + 
        fabsf(b1Extents.y * b01Mat._23) + 
        fabsf(b1Extents.z * b01Mat._33);

    float b1MinZ = b1CenterIn0.z - b1RadiiZ;
    float b1MaxZ = b1CenterIn0.z + b1RadiiZ;

    bool overlapZ =
        ((b0MinZ <= b1MinZ) && (b1MinZ <= b0MaxZ)) ||
        ((b0MinZ <= b1MaxZ) && (b1MaxZ <= b0MaxZ)) ||
        ((b1MinZ <= b0MinZ) && (b0MinZ <= b1MaxZ)) ||
        ((b1MinZ <= b0MaxZ) && (b0MaxZ <= b1MaxZ));

    //printf("BoxesOverlap: b0z=[%g,%g] b1z=[%g,%g] overlap=%d\n", b0MinZ, b0MaxZ, b1MinZ, b1MaxZ, overlapZ);

    return overlapZ;
}

inline bool matSegmentIntersectsSphere(const matVector3& s0Center, 
                                    float s0Radius2,
                                    const matVector3& pos, 
                                    const matVector3& v3Dir, 
                                    float distance)
{
    // trivial rejection - bounding sphere around segment vs. bounding
    // sphere of surface
    matVector3 deltaP0 = s0Center - pos;
    float deltaP0Mag = deltaP0.MagnitudeSquared();

    float sumRad = distance + s0Radius2;
    float sumRadSquared = sumRad * sumRad;
    if (deltaP0Mag > sumRadSquared)
    {
        // we will never intersect
        return false;
    }

    // test the line segment against the sphere
    matVector3 p1 = pos + v3Dir * distance;
    float rsquared = s0Radius2 * s0Radius2;
    // case 1: either endpoint is inside the sphere (trivial hit)

    matVector3 deltaP1 = s0Center - p1;
    if ( (deltaP1.MagnitudeSquared() <= rsquared) ||
        (deltaP0.MagnitudeSquared() <= rsquared) )
    {
        // one or both endpoints of the segment are inside the sphere
        return true;
    }

    // case 2: both endpoints are outside the sphere
    // create a vector from the center of the sphere to the start of the line segment
    matVector3 lineVector(v3Dir);
    lineVector *= distance;
    // try to determine the closest point on the line to the sphere center
    float32 fTheta = lineVector.DotProduct(deltaP0) / lineVector.DotProduct(lineVector);
    lineVector *= fTheta;
    matVector3 linePos(pos);
    linePos += lineVector;

    if ( rsquared < (s0Center - linePos).MagnitudeSquared() )
    {
        // closest point is outside the sphere
        return false;
    }

    return true;
}


#endif // INCLUDED_matIntersect_h


