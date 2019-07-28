/*****************************************************************************
	created:	2002/10/23
	copyright:	2002, NCSoft. All Rights Reserved
	author(s):	Jason Beardsley
	
	purpose:	Functions for computing distance between various objects.
*****************************************************************************/

#ifndef INCLUDED_matDistance_h
#define INCLUDED_matDistance_h

class matVector3;

namespace matDistance
{
  float PointToLineSquared(const matVector3& p,
                           const matVector3& linep0, const matVector3& linep1,
                           matVector3* pClosestPt = NULL);

  float PointToSegmentSquared(const matVector3& p,
                              const matVector3& segp0, const matVector3& segp1,
                              matVector3* pClosestPt = NULL);

  float LineToLineSquared(const matVector3& line0p0, const matVector3& line0p1,
                          const matVector3& line1p0, const matVector3& line1p1,
                          matVector3* line0pt = NULL, matVector3* line1pt = NULL);

  float SegmentToSegmentSquared(const matVector3& seg0p0, const matVector3& seg0p1,
                                const matVector3& seg1p0, const matVector3& seg1p1,
                                matVector3* seg0pt = NULL, matVector3* seg1pt = NULL);

  float PointToAlignedBoxSquared(const matVector3& p,
                                 const matVector3& bMin, const matVector3& bMax);

  float LineToAlignedBoxSquared(const matVector3& linep0, const matVector3& linep1,
                                const matVector3& bMin, const matVector3& bMax,
                                matVector3* pClosestPt = NULL);

  float SegmentToAlignedBoxSquared(const matVector3& segp0, const matVector3& segp1,
                                   const matVector3& bMin, const matVector3& bMax,
                                   matVector3* pClosestPt = NULL);

  float SegmentToAlignedBoxSquaredApprox(const matVector3& segp0, const matVector3& segp1,
                                         const matVector3& bMin, const matVector3& bMax,
                                         int iMaxIter, matVector3* pClosestPt = NULL);
}

#endif // INCLUDED_matDistance_h
