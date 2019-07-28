/*****************************************************************************
    created:    2001/09/25
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matFarPosition
#define   INCLUDED_matFarPosition
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTypes.h"
#include "arda2/math/matVector3.h"

class stoChunkFileReader;
class stoChunkFileWriter;
class utlPoint;
typedef utlPoint utlPoint;

#if CORE_COMPILER_MSVC
// suppress non-standard extension warnings from the windows headers
#pragma warning(disable: 4201)
#endif

struct matFarSegment
{
    int x, y, z;

    matFarSegment() {};
    matFarSegment( matZeroType ) : x(0), y(0), z(0) {};
    matFarSegment( IN int iX, IN int iY, IN int iZ) : x(iX), y(iY), z(iZ) {}

    void Set( IN int iX, IN int iY, IN int iZ) { x = iX; y = iY; z = iZ; }

    matVector3 Subtract( IN const matFarSegment& other ) const
    {
        return matVector3(
            static_cast<float>(x - other.x), 
            static_cast<float>(y - other.y), 
            static_cast<float>(z - other.z));
    }

    bool operator==( const matFarSegment& rhs ) const 
    { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
    bool operator!=( const matFarSegment& rhs ) const 
    { return (x != rhs.x) || (y != rhs.y) || (z != rhs.z); }
};

inline matVector3 GetInterSegmentTranslation( IN const matFarSegment& from, IN const matFarSegment& to)
{
    return from.Subtract(to);
}


#if CORE_COMPILER_MSVC
// renable non-standard extension warnings
#pragma warning(default: 4201)
#endif


class matFarPosition
{
public:
    matFarPosition() {};
    matFarPosition( matZeroType ) : m_segment(matZero), m_offset(matZero) {};
    matFarPosition( matFarSegment segment, const matVector3& offset ) : m_segment(segment), m_offset(offset) {};
    explicit matFarPosition( const matVector3& pos ) : m_segment(matZero), m_offset(pos) { Normalize(); }
    
    ~matFarPosition() {};

    void Normalize();

    const matFarSegment& GetSegment() const { return m_segment; }
    const matVector3& GetOffset() const { return m_offset; }
    float GetY() const { return m_segment.y + m_offset.y; }
    void SetY( float fY ) { m_segment.y = 0; m_offset.y = fY; Normalize();}

    // use of this function is discouraged - it introduces precision approximations
    // math calculations are best done in relative, not absolute, coordinate space
    matVector3 GetApproximateVector() const;

    void SetFromVector( const matVector3& vector )
    {
        m_segment.x = m_segment.y = m_segment.z = 0;
        m_offset = vector;
        Normalize();
    }

    matVector3 Subtract( const matFarPosition &pos ) const;

    matVector3 GetRelativeVector( const matFarSegment& segment ) const;

    void SetSegment( const matFarSegment& segment ) { m_segment = segment; }
    void SetOffset( const matVector3& offset ) { m_offset = offset; }
    void Set( const matFarSegment& segment, const matVector3& offset )
    {
        m_segment = segment;
        m_offset = offset;
    }

    void Translate( const matVector3& vector )
    {
        m_offset += vector;
        Normalize();
    }

    DEPRECATED void SetIntegerXZCoords( const utlPoint &p);
    DEPRECATED void SetIntegerXZCoords( int32 x, int32 y);
    DEPRECATED utlPoint GetIntegerXZCoords() const;
    void SetIntegerCoords( IN int x, IN int y, IN int z );
    void GetIntegerCoords( OUT int& x, OUT int& y, OUT int& z ) const;


    bool operator==( const matFarPosition& rhs ) const 
    { return Subtract(rhs).IsZero();}
    bool operator!=( const matFarPosition& rhs ) const 
    { return !Subtract(rhs).IsZero();}

    static matFarPosition Zero;
    friend class matFarPositionTests;

protected:
    // assignment operator okay
    // copy constructor okay
    
private:
    // assignment operator okay
    // copy constructor okay
    
    matFarSegment   m_segment;
    matVector3      m_offset;

    friend errResult Serialize( stoChunkFileWriter &rWriter, const matFarPosition &farpos );
    friend errResult Unserialize( stoChunkFileReader &rReader, matFarPosition &farpos  );
};


inline matVector3 operator-( const matFarPosition &lhs, const matFarPosition &rhs )
{
    return lhs.Subtract(rhs);
}


inline matVector3 matFarPosition::Subtract( const matFarPosition& pos ) const
{
    matVector3 r = m_offset - pos.m_offset;
    r.x += m_segment.x - pos.m_segment.x; 
    r.y += m_segment.y - pos.m_segment.y; 
    r.z += m_segment.z - pos.m_segment.z; 
    return r;
}


inline matVector3 matFarPosition::GetRelativeVector( const matFarSegment& segment ) const
{
    matVector3 r = m_offset;
    r.x += m_segment.x - segment.x; 
    r.y += m_segment.y - segment.y; 
    r.z += m_segment.z - segment.z; 
    return r;
}


inline const matFarPosition matLerp(const matFarPosition& Start,
                                    const matFarPosition& End,
                                    float frac)
{
    matVector3 dVector = End.Subtract(Start) * frac;
    matFarPosition newFarPos = Start;
    newFarPos.Translate(dVector);
    return newFarPos;
}

inline matVector3 GetInterSegmentTranslation( IN const matFarPosition& from, IN const matFarPosition& to)
{
    return GetInterSegmentTranslation(from.GetSegment(), to.GetSegment());
}



#endif // INCLUDED_matFarPosition
