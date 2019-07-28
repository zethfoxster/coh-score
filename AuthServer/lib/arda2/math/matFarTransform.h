/*****************************************************************************
    created:    2001/09/25
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matFarTransform
#define   INCLUDED_matFarTransform
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTypes.h"
#include "arda2/math/matQuaternion.h"
#include "arda2/math/matTransform.h"
#include "arda2/math/matFarPosition.h"

class matMatrix4x4;
class stoChunkFileReader;
class stoChunkFileWriter;

class matFarTransform
{
public:
    matFarTransform() : m_position(matZero), m_iChangeStamp(0) {};
    ~matFarTransform() {};

    void Set( const matFarSegment& segment, const matTransform& transform );

    void SetQuaternion( const matQuaternion& q ) { m_localTransform.SetQuaternion(q); ++m_iChangeStamp; }
    void SetScale( float fScale ) { m_localTransform.SetScale(fScale); ++m_iChangeStamp; }
    void SetScaleVector( matVector3 vScale ) { m_localTransform.SetScaleVector(vScale); ++m_iChangeStamp; }
    void SetPosition( const matFarPosition &p ) { m_position = p; m_localTransform.SetPosition(p.GetOffset()); ++m_iChangeStamp;}

    // accessors
    const matQuaternion&    GetQuaternion() const   { return m_localTransform.GetQuaternion(); }
    const matFarPosition&   GetPosition() const     { return m_position; }
    const float             GetScale() const        { return m_localTransform.GetScale(); }
    const float             GetInvScale() const     { return m_localTransform.GetInvScale(); }
    const matVector3&       GetScaleVector() const  { return m_localTransform.GetScaleVector(); }
    const matVector3&       GetInvScaleVector() const   { return m_localTransform.GetInvScaleVector();   }
    const matFarSegment&    GetSegment() const      { return m_position.GetSegment(); }
    uint32                  GetChangeStamp() const  { return m_iChangeStamp; }

    // get transforms and matrices in segment space
    const matMatrix4x4&     GetMatrix() const       { return m_localTransform.GetMatrix(); }
    const matMatrix4x4&     GetMatrixInverse() const { return m_localTransform.GetMatrixInverse(); }
    const matTransform&     GetLocalTransform() const   { return m_localTransform; }
    void                    GetRelativeMatrix(OUT matMatrix4x4& matrix, IN const matFarSegment& segment) const;

    const bool              IsNonUniformScale() const  { return m_localTransform.IsNonUniformScale(); }

    // conversion/compatibility
    matTransform GetApproximateNearTransform() const;

    // non-const versions allow frobbing
    matQuaternion&  ModifyQuaternion()  { ++m_iChangeStamp; return m_localTransform.ModifyQuaternion(); }

    // conversion from local space to world space
    matFarPosition TransformPoint( const matVector3& vOffset ) const;

    matVector3 TransformVectorWithScaling( const matVector3& vNormal ) const 
    { return m_localTransform.TransformVectorWithScaling(vNormal); }

    matVector3 TransformVectorNoScaling( const matVector3& vNormal ) const 
    { return m_localTransform.TransformVectorNoScaling(vNormal); }

    // convert from world space into local space
    matVector3 InverseTransformPoint( const matFarPosition& position ) const;

    // pre-multiply a local transform
    matFarTransform& PreTransform( const matTransform& lhs );

    void Translate( const matVector3& vOffsetWorld );
    void Rotate( const matQuaternion &q );
    void RotateEuler( float fYaw, float fPitch, float fRoll );
    void LocalTranslate( const matVector3& vOffsetLocal );
    void LocalRotate( const matQuaternion &q );
    void LocalRotateEuler( float fYaw, float fPitch, float fRoll );

    void SetEulerAngles( float fYaw, float fPitch, float fRoll );

    matFarTransform& operator=( const matFarTransform& rhs )
    {
        m_position = rhs.m_position;
        m_localTransform = rhs.m_localTransform;

        ++m_iChangeStamp;
        return *this;
    }

protected:
    
private:
    matFarPosition          m_position;
    mutable matTransform    m_localTransform;
    uint32                  m_iChangeStamp;

    friend errResult Serialize( stoChunkFileWriter &writer, const matFarTransform& farpos );
    friend errResult Unserialize( stoChunkFileReader &reader, matFarTransform& farpos );
};

inline void matFarTransform::GetRelativeMatrix(matMatrix4x4& matrix, const matFarSegment& segment) const
{
    matrix = GetMatrix();
    matrix.GetRow3() += GetSegment().Subtract(segment);
}

/**
 *  Initialize
 */
inline void matFarTransform::Set( const matFarSegment& segment, const matTransform& transform )
{
    m_localTransform = transform;
    m_position.SetSegment(segment);
    m_position.SetOffset(transform.GetPosition());

    ++m_iChangeStamp;
}


// TODO[pmf]: create ComputeSimilarityTransform()
inline matMatrix4x4 ComputeSimilarityMatrix( const matFarTransform& xFrom, const matFarTransform& xTo )
{
    // matrix to convert from xFrom's local space to xFrom's segment space
    matMatrix4x4 sim(xFrom.GetMatrix());

    // adjust to convert to xTO's segment space
    sim.GetPosition() += xFrom.GetSegment().Subtract(xTo.GetSegment());

    // now convert to xTo's local space
    sim.Multiply(xTo.GetMatrixInverse());

    return sim;
}

inline matVector3 GetInterSegmentTranslation( IN const matFarTransform& from, IN const matFarTransform& to)
{
    return GetInterSegmentTranslation(from.GetSegment(), to.GetSegment());
}



#endif // INCLUDED_matFarTransform
