/*****************************************************************************
	created:	2001-07-25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter Freese, Tom Gambill
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matTransform.h"


//*****************************************************************
//*  matTransform()
//*
//*
//*****************************************************************
matTransform::matTransform() :
	m_matrix(matIdentity),
	m_matrixInverse(matIdentity),
	m_quaternion(matIdentity),
	m_position(matZero),
    m_vScale(1.0f,1.0f,1.0f),
    m_vInvScale(1.0f,1.0f,1.0f),
	m_iChangeStamp(0),
    m_bNonUniformScale(false)
{
}

const matMatrix4x4& matTransform::UpdateMatrix() const
{
    m_matrix.StoreTransformation(m_vScale, m_quaternion, m_position);
    
    m_dirtyFlags.Clear(kMatrix);
	return m_matrix;
}


const matMatrix4x4& matTransform::UpdateMatrixInverse() const
{
    if ( IsNonUniformScale() )
        m_matrixInverse.StoreInverse(GetMatrix());
    else
	    m_matrixInverse.StoreInverseFast(GetMatrix());
	m_dirtyFlags.Clear(kMatrixInverse);

	return m_matrixInverse;
}


const matVector3& matTransform::UpdateInvScale() const
{
	if ( IsNonUniformScale() )
	{
		m_vInvScale.x = 1.0f / m_vScale.x;
		m_vInvScale.y = 1.0f / m_vScale.y;
		m_vInvScale.z = 1.0f / m_vScale.z;
	}
	else
	{
		m_vInvScale.x = m_vInvScale.y = m_vInvScale.z = 1.0f / m_vScale.x;
	}
	m_dirtyFlags.Clear(kInvScale);
	return m_vInvScale;
}


matTransform& matTransform::PreTransform( const matTransform& lhs )
{
    m_position = TransformPoint(lhs.GetPosition());
    m_quaternion.PreMultiply(lhs.GetQuaternion());
    m_vScale *= lhs.GetScaleVector();
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
    return *this;
}


matTransform& matTransform::PostTransform( const matTransform& rhs )
{
    m_position += rhs.GetPosition();
    m_quaternion.Multiply(rhs.GetQuaternion());
    m_vScale *= rhs.GetScaleVector();
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
    return *this;
}



matVector3 matTransform::TransformPoint( const matVector3& vIn ) const
{
	matVector3 v;
	v.StoreTransformPoint(GetMatrix(), vIn);
	return v;
}

// Deprecated in favor of TransformVectorNoScaling
matVector3 matTransform::TransformVector( const matVector3& vIn ) const
{
    return TransformVectorNoScaling( vIn );
}

matVector3 matTransform::TransformVectorNoScaling( const matVector3& vIn ) const
{
    matVector3 v;
    v.StoreTransformVector(GetMatrix(), vIn);
    v *= GetInvScaleVector();
    return v;
}


matVector3 matTransform::TransformVectorWithScaling( const matVector3& vIn ) const
{
    matVector3 v;
    v.StoreTransformVector(GetMatrix(), vIn);
    return v;
}

matVector3 matTransform::InverseTransformPoint( const matVector3& vIn ) const
{
	matVector3 v;
	v.StoreTransformPoint(GetMatrixInverse(), vIn);
	return v;
}

// Deprecated in favor of InverseTransformVectorNoScaling
matVector3 matTransform::InverseTransformVector( const matVector3& vIn ) const
{
    return InverseTransformVectorNoScaling( vIn );
}

matVector3 matTransform::InverseTransformVectorNoScaling( const matVector3& vIn ) const
{
    matVector3 v;
    v.StoreTransformVector(GetMatrixInverse(), vIn);
    v *= GetScaleVector();
    return v;
}

matVector3 matTransform::InverseTransformVectorWithScaling( const matVector3& vIn ) const
{
    matVector3 v;
    v.StoreTransformVector(GetMatrixInverse(), vIn);
    return v;
}


/**
 *  Adds a world-space translation to this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of translating 
 *  the object in world-space regardless of its orientation.
 *
 *  @param vDeltaWorld World space translation to apply
 */
void matTransform::Translate( const matVector3& vDeltaWorld )
{
    m_position += vDeltaWorld;
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


/**
 *  Performs an in-place world-space rotation on this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of rotating 
 *  the object about the world-space axes but with the rotation centered about its local
 *  origin.
 *
 *  @param qRotation World-axes based rotation to apply
 */
void matTransform::Rotate( const matQuaternion &qRotation )
{
    m_quaternion.Multiply(qRotation);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


/**
 *  Performs an in-place world-space rotation on this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of rotating 
 *  the object about the world-space axes but with the rotation centered about its local
 *  origin.
 *
 *  @param fYaw      Amount of rotation about the World y-axis
 *  @param fPitch    Amount of rotation about the World x-axis
 *  @param fRoll     Amount of rotation about the World z-axis
 */
void matTransform::RotateEuler( float fYaw, float fPitch, float fRoll )
{
    matQuaternion q;
    q.StoreEulerAngles(fYaw, fPitch, fRoll);
    m_quaternion.Multiply(q);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


/**
 *  Adds a local-space translation to this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of translating 
 *  the object in local-space according to its orientation.
 *
 *  @param vDeltaLocal Local space translation to apply
 */
void matTransform::LocalTranslate( const matVector3 &vDeltaLocal )
{
    m_position += TransformVectorNoScaling(vDeltaLocal);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


/**
 *  Performs an in-place local-space rotation on this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of rotating 
 *  the object about its local-space axes with the rotation centered about its local
 *  origin.
 *
 *  @param qRotation Local-axes based rotation to apply
 */
void matTransform::LocalRotate( const matQuaternion &qRotation )
{
    m_quaternion.PreMultiply(qRotation);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


/**
 *  Performs an in-place local-space rotation on this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of rotating 
 *  the object about its local-space axes with the rotation centered about its local
 *  origin.
 *
 *  @param fYaw      Amount of rotation about the Local y-axis
 *  @param fPitch    Amount of rotation about the Local x-axis
 *  @param fRoll     Amount of rotation about the Local z-axis
 */
void matTransform::LocalRotateEuler( float fYaw, float fPitch, float fRoll )
{
    matQuaternion q;
    q.StoreEulerAngles(fYaw, fPitch, fRoll);
    m_quaternion.PreMultiply(q);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}

/**
 *  Directly set the orientation of this transform
 *  
 *  @param fYaw      Amount of rotation about the World y-axis
 *  @param fPitch    Amount of rotation about the World x-axis
 *  @param fRoll     Amount of rotation about the World z-axis
 */
void matTransform::SetEulerAngles( float fYaw, float fPitch, float fRoll )
{
    m_quaternion.StoreEulerAngles(fYaw, fPitch, fRoll);
    m_dirtyFlags.SetAll();
    ++m_iChangeStamp;
}


