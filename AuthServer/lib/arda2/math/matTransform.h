/**
 *  Basic wrapper for 3D transformations
 *
 *  Copyright (c) 2003, NCsoft. All Rights Reserved
 *
 *  @par Last modified
 *      $DateTime: 2007/12/12 10:43:51 $
 *      $Author: pflesher $
 *      $Change: 700168 $
 *  @file
 */

#ifndef   INCLUDED_matTransform
#define   INCLUDED_matTransform
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matQuaternion.h"
#include "arda2/math/matMatrix4x4.h"
#include "arda2/util/utlBitFlag.h"

class stoChunkFileReader;
class stoChunkFileWriter;
const float XFM_SCALE_COMPARE_EPSILON = 0.00001f;


/**
 *  @ingroup arda2
 *  @brief 3D transformation
 *  
 *  This class encapsulates a 3D transformation in local space. It will generate a matrix and
 *  matrix inverse on demand, supports serialization and change monitoring.
 *  
 *  @note Although this class currently maintains its internal state as a quaternion, offset,
 *  and scale, at some point in the future it will probably simply keep state in a matrix for
 *  performance reasons.
 */
class matTransform 
{
public:
    matTransform();
    ~matTransform() {};

    /**
     *  Set the orientation for a transformation. Non-orientation components, such as scale and position
     *  are unaltered.
     *
     *  @param q New orientation
     */
    void SetQuaternion( const matQuaternion& q )
    {
        if (q != m_quaternion)
        {
            m_quaternion = q;
            m_dirtyFlags.SetAll();
            ++m_iChangeStamp;
        }
    }

    /**
     *  Set the position for a transformation. Non-position components, such as orientation and scale 
     *  are unaltered.
     *
     *  @param v New position
     */
    void SetPosition( const matVector3 &v )
    {
        if (v != m_position)
        {
            m_position = v;
            if (!m_dirtyFlags.Test(kMatrix))
            {
                // matrix is 'clean' so just update its translational components
                m_matrix._41 = v.x;
                m_matrix._42 = v.y;
                m_matrix._43 = v.z;
                m_dirtyFlags.Set(kMatrixInverse); // dirty the inverse
            }
            ++m_iChangeStamp;
        }
    }

    /**
     *  Set the uniform scale for a transformation. Non-scale components, such as 
     *  orientation and position are unaltered.
     *
     *  @param f New scalar scale
     */
    void SetScale( float f )
    {
        if (f != m_vScale.x && f != m_vScale.y && f != m_vScale.z)
        {
            m_vScale.x = m_vScale.y = m_vScale.z = f;
            m_dirtyFlags.SetAll();
            m_bNonUniformScale = false;
            ++m_iChangeStamp;
        }
    }

    /**
     *  Set the non-uniform scale for a transformation. Non-scale components are
     *  unaltered.
     *
     *  @param s New vector scale
     */
    void SetScaleVector( matVector3 s )
    {
        if (s != m_vScale)
        {
            m_vScale = s;
            m_dirtyFlags.SetAll();
            if (((fabsf(m_vScale.x - m_vScale.y) > XFM_SCALE_COMPARE_EPSILON) ||
                (fabsf(m_vScale.x - m_vScale.z) > XFM_SCALE_COMPARE_EPSILON)))
                m_bNonUniformScale = true;
            else
                m_bNonUniformScale = false;
            ++m_iChangeStamp;
        }
    }

    const matQuaternion&    GetQuaternion() const   { return m_quaternion; }
    const matVector3&       GetPosition() const     { return m_position; }
    const float             GetScale() const        
    {
        errAssert(!IsNonUniformScale());
        return m_vScale.x; 
    }

    const matVector3&       GetScaleVector() const  { return m_vScale; }
    const matMatrix4x4&     GetMatrix() const       { return m_dirtyFlags.Test(kMatrix) ? UpdateMatrix() : m_matrix; }
    const matMatrix4x4&     GetMatrixInverse() const { return m_dirtyFlags.Test(kMatrixInverse) ? UpdateMatrixInverse() : m_matrixInverse; }
    const float             GetInvScale() const     
    { 
        errAssert(!IsNonUniformScale());
        return m_dirtyFlags.Test(kInvScale) ? UpdateInvScale().x : m_vInvScale.x;
    }

    const matVector3&       GetInvScaleVector() const { return m_dirtyFlags.Test(kInvScale) ? UpdateInvScale() : m_vInvScale; }
    uint32                  GetChangeStamp() const {return m_iChangeStamp;}

    bool                    IsNonUniformScale() const  { return m_bNonUniformScale; }

    // non-const versions allow frobbing
    matQuaternion&  ModifyQuaternion()  { m_dirtyFlags.SetAll(); ++m_iChangeStamp; return m_quaternion; }
    matVector3&     ModifyPosition()    { m_dirtyFlags.SetAll(); ++m_iChangeStamp; return m_position; }

    matTransform&   PreTransform( const matTransform& lhs );
    matTransform&   PostTransform( const matTransform& rhs );

    void Translate( const matVector3& vOffsetWorld );
    void Rotate( const matQuaternion &q );
    void RotateEuler( float fYaw, float fPitch, float fRoll );
    void LocalTranslate( const matVector3& vOffsetLocal );
    void LocalRotate( const matQuaternion &q );
    void LocalRotateEuler( float fYaw, float fPitch, float fRoll );

    void SetEulerAngles( float fYaw, float fPitch, float fRoll );

    // following transformations account correctly for scale
    // use instead of transforming directly with matrix
    matVector3      TransformPoint( const matVector3& ) const;
    
    // TransformVectorNoScaling should be called instead
    // Renaming for clarity since TransformVector undoes scaling
    DEPRECATED matVector3      TransformVector( const matVector3& ) const;
    
    matVector3      TransformVectorNoScaling( const matVector3& ) const;
    matVector3      TransformVectorWithScaling( const matVector3& vIn ) const;
    matVector3      InverseTransformPoint( const matVector3& ) const;
    
    // InverseTransformVectorNoScaling should be called instead
    // Renaming for clarity since InverseTransformVector undoes scaling
    DEPRECATED matVector3      InverseTransformVector( const matVector3& ) const;
    
    matVector3      InverseTransformVectorNoScaling( const matVector3& ) const;
    matVector3      InverseTransformVectorWithScaling( const matVector3& vIn ) const;

    matTransform& operator=( const matTransform& rhs )
    {
        m_quaternion = rhs.m_quaternion; 
        m_position = rhs.m_position;
        m_vScale = rhs.m_vScale;
		m_bNonUniformScale = rhs.m_bNonUniformScale;
        m_dirtyFlags = rhs.m_dirtyFlags;
        if ( !m_dirtyFlags.Test(kMatrix) )
            m_matrix = rhs.m_matrix;
        if ( !m_dirtyFlags.Test(kMatrixInverse) )
            m_matrixInverse = rhs.m_matrixInverse;
        if ( !m_dirtyFlags.Test(kInvScale) )
            m_vInvScale = rhs.m_vInvScale;
        ++m_iChangeStamp;
        return *this;
    }

protected:

    const matMatrix4x4& UpdateMatrix() const;
    const matMatrix4x4& UpdateMatrixInverse() const;
    const matVector3& UpdateInvScale() const;

private:
    mutable matMatrix4x4    m_matrix;
    mutable matMatrix4x4    m_matrixInverse;

    matQuaternion           m_quaternion;
    matVector3              m_position;
    matVector3              m_vScale;
    mutable matVector3      m_vInvScale;
    uint32                  m_iChangeStamp;
    bool                    m_bNonUniformScale;

    enum eFlags
    {
        kMatrix,
        kMatrixInverse,
        kInvScale,
    };

    mutable utlBitFlag<eFlags> m_dirtyFlags;

    friend errResult Unserialize( stoChunkFileReader &rReader, matTransform& ws );
    friend errResult Serialize( stoChunkFileWriter &rWriter, const matTransform& ws );
};


// TODO[pmf]: create ComputeSimilarityTransform()
inline matMatrix4x4 ComputeSimilarityMatrix( const matTransform& xFrom, const matTransform& xTo )
{
    // matrix to convert from xFrom's space to xTo's space
    return xFrom.GetMatrix() * xTo.GetMatrixInverse();
}


#endif // INCLUDED_matTransform
