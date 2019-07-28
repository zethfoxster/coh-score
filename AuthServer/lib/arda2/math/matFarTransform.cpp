/*****************************************************************************
	created:	2001/09/28
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#include "arda2/math/matFirst.h"
#include "arda2/math/matFarTransform.h"
#include "arda2/math/matMatrix4x4.h"

/**
 *  Transforms the provided point from local space into world space represented as a matFarPosition
 *
 *  @param vOffset Point in local space
 *
 *  @return Far position representation of the transformed point in world space
 */
matFarPosition matFarTransform::TransformPoint( const matVector3& vOffset ) const
{
    matFarPosition pos(m_position.GetSegment(), GetLocalTransform().TransformPoint(vOffset));
    pos.Normalize();
    return pos;
}


/**
*  Transforms the provided world space position into local space
*
*  @param position Point in world space
*
*  @return matVector3 representing position in local space
*/
matVector3 matFarTransform::InverseTransformPoint( const matFarPosition& position ) const
{
    matVector3 vRelPosition = position.GetRelativeVector(GetSegment());
    return GetLocalTransform().InverseTransformPoint(vRelPosition);
}


/**
 *  Prepends a local transformation to the far transformation.  This can be conceptualized
 *  as performing the provided transformation in this transform's space. For example, if this
 *  transform is associated with an object, and the transform provided represents a rotation 
 *  about the y-axis, then this operation has the effect of rotating the object around its
 *  local y-axis.
 *
 *  @param lhs Local space transform to apply
 *  
 *  @return Reference to self
 */
matFarTransform& matFarTransform::PreTransform( const matTransform& lhs )
{
    m_position = TransformPoint(lhs.GetPosition());
    ModifyQuaternion().PreMultiply(lhs.GetQuaternion());
    SetScale(lhs.GetScale() * GetScale());
    m_localTransform.SetPosition(m_position.GetOffset());
    ++m_iChangeStamp;
    return *this;
}


/**
 *  Adds a world-space translation to this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of translating 
 *  the object in world-space regardless of its orientation.
 *
 *  @param vDeltaWorld World space translation to apply
 */
void matFarTransform::Translate( const matVector3& vDeltaWorld )
{
    m_position.Translate(vDeltaWorld);
    m_localTransform.SetPosition(m_position.GetOffset());
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
void matFarTransform::Rotate( const matQuaternion &qRotation )
{
    m_localTransform.ModifyQuaternion().Multiply(qRotation);
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
void matFarTransform::RotateEuler( float fYaw, float fPitch, float fRoll )
{
    matQuaternion q;
    q.StoreEulerAngles(fYaw, fPitch, fRoll);
    m_localTransform.ModifyQuaternion().Multiply(q);
    ++m_iChangeStamp;
}


/**
 *  Adds a local-space translation to this transform.  For example, if this
 *  transform is associated with an object, this operation has the effect of translating 
 *  the object in local-space according to its orientation.
 *
 *  @param vDeltaLocal Local space translation to apply
 */
void matFarTransform::LocalTranslate( const matVector3 &vDeltaLocal )
{
    m_position.Translate(GetLocalTransform().TransformVectorNoScaling(vDeltaLocal));
    m_localTransform.SetPosition(m_position.GetOffset());
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
void matFarTransform::LocalRotate( const matQuaternion &qRotation )
{
    m_localTransform.ModifyQuaternion().PreMultiply(qRotation);
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
void matFarTransform::LocalRotateEuler( float fYaw, float fPitch, float fRoll )
{
    matQuaternion q;
    q.StoreEulerAngles(fYaw, fPitch, fRoll);
    m_localTransform.ModifyQuaternion().PreMultiply(q);
    ++m_iChangeStamp;
}

/**
 *  Directly set the orientation of this transform
 *  
 *  @param fYaw      Amount of rotation about the World y-axis
 *  @param fPitch    Amount of rotation about the World x-axis
 *  @param fRoll     Amount of rotation about the World z-axis
 */
void matFarTransform::SetEulerAngles( float fYaw, float fPitch, float fRoll )
{
	m_localTransform.ModifyQuaternion().StoreEulerAngles(fYaw, fPitch, fRoll);
    ++m_iChangeStamp;
}


matTransform matFarTransform::GetApproximateNearTransform() const
{
    matTransform nearTransform;
    nearTransform.SetScaleVector(GetScaleVector());
    nearTransform.SetQuaternion(GetQuaternion());
    nearTransform.SetPosition(GetPosition().GetApproximateVector());
    return nearTransform;
}

