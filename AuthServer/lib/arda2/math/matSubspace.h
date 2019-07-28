/**
 *  copyright:  (c) 2004, NCsoft. All Rights Reserved
 *  author(s):  Peter Freese
 *  purpose:    
 *  modified:   $DateTime: 2007/12/12 10:43:51 $
 *              $Author: pflesher $
 *              $Change: 700168 $
 *              @file
 */

#ifndef   INCLUDED_matSubspace
#define   INCLUDED_matSubspace
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matPlane.h"


enum eCullResult
{
    kCullNone       = 0x00000000,
    kCullPartial    = 0x00000001,
    kCullAll        = 0xFFFFFFFF,
};
typedef eCullResult eCullFlag;  // backwards compatibility


class matSubspace
{
public:
    matSubspace() {};
    ~matSubspace() {};

    int GetNumPlanes() const { return (int)m_planes.size(); }
    void SetNumPlanes( int nPlanes ) { m_planes.resize(nPlanes); }

    void Clear() { m_planes.clear(); }

    const matPlane& GetPlane( int nPlane ) const { return m_planes[nPlane]; }
    const matPlane* GetPlanes() const 
    { 
        if (m_planes.empty())
            return NULL;
        return &m_planes[0]; 
    }

    matPlane& ModifyPlane( int nPlane ) { return m_planes[nPlane]; }
    matPlane* ModifyPlanes() 
    { 
        if (m_planes.empty())
            return NULL;
        return &m_planes[0]; 
    }
    
    eCullResult TestSphereCull( const matVector3& center, float fRadius ) const
    {
        eCullResult cullResult = kCullNone;
        for ( uint nPlane = 0; nPlane < m_planes.size(); ++nPlane )
        {
            float fDist = m_planes[nPlane].GetSignedDistance(center);
            if ( fDist < -fRadius )
            {
                return kCullAll;
            }

            if ( fDist < fRadius )
                cullResult = kCullPartial;
        }

        return cullResult;
    }

private:
    std::vector<matPlane>    m_planes;
};


#endif // INCLUDED_matSubspace


