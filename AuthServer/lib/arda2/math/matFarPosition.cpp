/*****************************************************************************
	created:	2001/09/25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matFarPosition.h"
#include "arda2/util/utlPoint.h"

#define NORMALIZATION	1
matFarPosition matFarPosition::Zero(matZero);

void matFarPosition::Normalize()
{
#if NORMALIZATION
    if ( fabsf(m_offset.x) >= 1.0f )
    {
        int intPart = matFloatToInt(m_offset.x);
        m_segment.x += intPart;
        m_offset.x -= intPart;
    }
    if ( fabsf(m_offset.y) >= 1.0f )
    {
        int intPart = matFloatToInt(m_offset.y);
        m_segment.y += intPart;
        m_offset.y -= intPart;
    }
    if ( fabsf(m_offset.z) >= 1.0f )
    {
        int intPart = matFloatToInt(m_offset.z);
        m_segment.z += intPart;
        m_offset.z -= intPart;
    }
#endif
}


matVector3 matFarPosition::GetApproximateVector() const
{
    return matVector3(m_segment.x + m_offset.x, m_segment.y + m_offset.y, m_segment.z + m_offset.z);
}

// TODO[pmf]: Add these functions to matFarPosition unit tests
void matFarPosition::SetIntegerXZCoords( const utlPoint &p )
{
    m_segment.x = p.x;
    m_segment.y = 0;
    m_segment.z = p.y;
    m_offset.x = m_offset.y = m_offset.z = 0.0f;
}

void matFarPosition::SetIntegerXZCoords(int32 x, int32 y)
{
    m_segment.x = x;
    m_segment.y = 0;
    m_segment.z = y;
    m_offset.x = m_offset.y = m_offset.z = 0.0f;
}

utlPoint matFarPosition::GetIntegerXZCoords() const
{
    return utlPoint(matFloatToInt(m_segment.x + m_offset.x), matFloatToInt(m_segment.z + m_offset.z));
}

void matFarPosition::SetIntegerCoords( IN int x, IN int y, IN int z )
{
    m_segment.x = x;
    m_segment.y = y;
    m_segment.z = z;
    m_offset = matVector3::Zero;
}

void matFarPosition::GetIntegerCoords( OUT int& x, OUT int& y, OUT int& z ) const
{
    x = matFloatToIntRound(m_segment.x + m_offset.x);
    y = matFloatToIntRound(m_segment.y + m_offset.y);
    z = matFloatToIntRound(m_segment.z + m_offset.z);
}

