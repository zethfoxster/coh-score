/*****************************************************************************
	created:	2001/09/25
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese
	
	purpose:	
*****************************************************************************/

#ifndef   INCLUDED_matMatrix4x4
#define   INCLUDED_matMatrix4x4
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matTypes.h"
#include "arda2/math/matVector2.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matVector4.h"
#include "arda2/math/matPlane.h"
#include "arda2/math/matQuaternion.h"

class matQuaternion;
class matMatrix4x4T;

// __declspec(align(16))
class matMatrix4x4 : public matBaseMATRIX
{
public:
	matMatrix4x4();
	matMatrix4x4( matIdentityType );
	explicit matMatrix4x4( const matBaseMATRIX &m ) : matBaseMATRIX(m) { }
	matMatrix4x4( const matMatrix4x4 &m );
	matMatrix4x4( float _11, float _12, float _13, float _14,
		float _21, float _22, float _23, float _24,
		float _31, float _32, float _33, float _34,
		float _41, float _42, float _43, float _44 );
    ~matMatrix4x4() {PERFSIZER_REMOVEOBJECT();}


	// FIXME[pmf]: Hmm... these methods are misleadingly named, as they DON'T return the row, just part of the orientation submatrix
	const matVector3&	GetRow0() const;
	const matVector3&	GetRow1() const;
	const matVector3&	GetRow2() const;
	const matVector3&	GetRow3() const;

	matVector3&			GetRow0();
	matVector3&			GetRow1();
	matVector3&			GetRow2();
	matVector3&			GetRow3();
	
	const matVector3&	GetPosition() const;
	matVector3&			GetPosition();
	void				SetPosition( IN const matVector3& v3Position );

	bool				IsIdentity() const;

	matMatrix4x4&		StoreIdentity();

	matMatrix4x4&		StoreRotationX( float fAngle );
	matMatrix4x4&		StoreRotationY( float fAngle );
	matMatrix4x4&		StoreRotationZ( float fAngle );
	matMatrix4x4&		StoreRotationAxis( const matVector3& Axis, float Angle );
	matMatrix4x4&		StoreEulerAngles( float fYaw, float fPitch, float fRoll );
	matMatrix4x4&		StoreQuaternion( const matQuaternion &rOrientation );
	matMatrix4x4&		StoreReflect( const matPlane& ReflectPlane );
	matMatrix4x4&		StoreScaling( float sX, float sY, float sZ );
	matMatrix4x4&		StoreTranspose( IN const matMatrix4x4 &mat );
	matMatrix4x4&		StoreInverse( IN const matMatrix4x4 &mat, OUT float *pfDet = NULL );
	matMatrix4x4&		StoreInverseFast( IN const matMatrix4x4 &mat );

	matMatrix4x4&		StoreAffineTransform( float scaling, const matVector3& RotationCenter, const matQuaternion& Rotation, 
							const matVector3& Translation );
	matMatrix4x4&		StoreTranslation( const matVector3& Translation );
	matMatrix4x4&		StoreTranslation2D( const matVector2& Translation );
	matMatrix4x4&		StoreLookDir( const matVector3 &vForward, const matVector3 &Up );
	matMatrix4x4&		StorePlaneProject( const matVector4& LightVec, const matPlane& Plane );
    matMatrix4x4&		StoreTransformation( const matVector3& ScaleCenter, const matQuaternion& ScalingRotation, 
                                             const matVector3& Scaling,
                                             const matVector3& RotationCenter, const matQuaternion& Rotation,
                                             const matVector3& Translation );
    matMatrix4x4&		StoreTransformation( const matVector3& Scaling,
                                             const matQuaternion& Rotation,
                                             const matVector3& Translation );

	// texture coordinate transform matrices
	matMatrix4x4&		StoreUVTransformation( 
                            const matVector2 &Scaling, 
							const matVector3 &Rotation,
							const matVector2 &Translation );

    // projection matrices
	void	StoreOrthographicLH( float Width, float Height, float MinZ, float MaxZ );
	void	StoreOrthographicRH( float Width, float Height, float MinZ, float MaxZ );
	void	StoreOrthographicOffCenterLH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ );
	void	StoreOrthographicOffCenterRH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ );

	void	StorePerspectiveLH(float Width, float Height, float MinZ, float MaxZ );
	void	StorePerspectiveRH(float Width, float Height, float MinZ, float MaxZ );
	void	StorePerspectiveFovLH( float FovY, float Aspect, float MinZ, float MaxZ );
	void	StorePerspectiveFovRH( float FovY, float Aspect, float MinZ, float MaxZ );
	void	StorePerspectiveOffCenterLH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ );
	void	StorePerspectiveOffCenterRH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ );

	matVector3	TransformPoint( IN const matVector3& v3In ) const;
	matVector3	TransformVector( IN const matVector3& v3In ) const;
	matVector4	Transform( IN const matVector4& v4In ) const;

	matMatrix4x4&	Multiply( IN const matMatrix4x4 &mat );
    matMatrix4x4&	PreMultiply( IN const matMatrix4x4 &mat );
	matMatrix4x4&	StoreMultiply( IN const matMatrix4x4 &mat1, IN const matMatrix4x4 &mat2 );
	matMatrix4x4&	StoreMultiplyTranspose(IN const matMatrix4x4 &mat1, IN const matMatrix4x4 &mat2 );

	matMatrix4x4&	Transpose();
	matMatrix4x4&	Invert( OUT float *pfDet = NULL );
	matMatrix4x4&	InvertFast();
	float		CalcDeterminant() const;
	void		ConvertRHtoLH();

	void		GetEulerAngles(  float &fYaw, float &fPitch, float &fRoll ) const;
	void		GetEulerAnglesNoRoll( float &fYaw, float &fPitch ) const;

	static const matMatrix4x4 Identity;

	friend matMatrix4x4 operator*( const matMatrix4x4&, const matMatrix4x4& );
};

//typedef __declspec(align(16)) class matMatrix4x4u matMatrix4x4;

// inlines
inline matMatrix4x4::matMatrix4x4() : matBaseMATRIX()
{
    PERFSIZER_ADDOBJECT();
}

inline matMatrix4x4::matMatrix4x4( const matMatrix4x4 &m ) : matBaseMATRIX(m)
{
    PERFSIZER_ADDOBJECT();
}

inline matMatrix4x4::matMatrix4x4( matIdentityType ) :
matBaseMATRIX(
		   1,0,0,0,
		   0,1,0,0,
		   0,0,1,0,
		   0,0,0,1)
{
    PERFSIZER_ADDOBJECT();
}

inline matMatrix4x4::matMatrix4x4( float _11, float _12, float _13, float _14,
			 float _21, float _22, float _23, float _24,
			 float _31, float _32, float _33, float _34,
			 float _41, float _42, float _43, float _44 ) : 
matBaseMATRIX( _11, _12, _13, _14,
		   _21, _22, _23, _24,
		   _31, _32, _33, _34,
		   _41, _42, _43, _44 ) {PERFSIZER_ADDOBJECT();};

inline const matVector3& matMatrix4x4::GetRow0() const
{
	return *((matVector3*)(&_11));
}

inline const matVector3& matMatrix4x4::GetRow1() const
{
	return *((matVector3*)(&_21));
}

inline const matVector3& matMatrix4x4::GetRow2() const
{
	return *((matVector3*)(&_31));
}

inline const matVector3& matMatrix4x4::GetRow3() const
{
	return *((matVector3*)(&_41));
}

inline matVector3& matMatrix4x4::GetRow0()
{
	return *((matVector3*)(&_11));
}

inline matVector3& matMatrix4x4::GetRow1() 
{
	return *((matVector3*)(&_21));
}

inline matVector3& matMatrix4x4::GetRow2()
{
	return *((matVector3*)(&_31));
}

inline matVector3& matMatrix4x4::GetRow3()
{
	return *((matVector3*)(&_41));
}

inline const matVector3& matMatrix4x4::GetPosition() const
{
	return *((matVector3*)(&_41));
}

inline matVector3& matMatrix4x4::GetPosition()
{
	return *((matVector3*)(&_41));
}

inline void matMatrix4x4::SetPosition( IN const matVector3& v3Position )
{
	*((matVector3*)(&_41)) = v3Position;
}

inline matMatrix4x4& matMatrix4x4::Transpose()
{
	matMatrix4x4 temp(*this);
	return StoreTranspose(temp);
}

inline bool matMatrix4x4::IsIdentity() const
{
	return matBaseMatrixIsIdentity(this) != FALSE;
}

inline matMatrix4x4& matMatrix4x4::StoreIdentity()
{
	matBaseMatrixIdentity(this);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreRotationX( float fAngle )
{
	matBaseMatrixRotationX(this,  fAngle);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreRotationY( float fAngle ) 
{
	matBaseMatrixRotationY(this,  fAngle);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreRotationZ( float fAngle ) 
{
	matBaseMatrixRotationZ(this,  fAngle);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreRotationAxis( const matVector3& Axis, float Angle )
{
	matBaseMatrixRotationAxis(this, &Axis, Angle);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreEulerAngles( float fYaw, float fPitch, float fRoll ) 
{
	matBaseMatrixRotationYawPitchRoll(this, fYaw, fPitch, fRoll);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreQuaternion( const matQuaternion &rOrientation ) 
{
	matBaseMatrixRotationQuaternion(this, &rOrientation);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreReflect( IN const matPlane& ReflectPlane )
{
	matBaseMatrixReflect(this, &ReflectPlane);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreScaling( float sX, float sY, float sZ )
{
	matBaseMatrixScaling(this, sX, sY, sZ);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreTranspose( IN const matMatrix4x4 &mat )
{
	matBaseMatrixTranspose(this, &mat);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreInverse( const matMatrix4x4 &mat, float *pfDet )
{
	matBaseMatrixInverse(this, pfDet, &mat);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreInverseFast( IN const matMatrix4x4 &mat )
{
	float fScale2 = mat.GetRow0().MagnitudeSquared();
	bool bNormal = fabs(fScale2 - 1.0f) < 0.0001f;
//	errAssertV( fabs(fScale2 - 1.0f) < 0.01f,("Cannot call InvertFast() on matrix with scaling"));
	StoreTranspose(mat);
	_14 = 0.0f;
	_24 = 0.0f;
	_34 = 0.0f;

	if ( !bNormal )
	{
		float fInvScale2 = 1.0f / fScale2;
		GetRow0() *= fInvScale2;
		GetRow1() *= fInvScale2;
		GetRow2() *= fInvScale2;
	}

	SetPosition(TransformVector(-mat.GetPosition()));
	return *this;
}

inline void	matMatrix4x4::ConvertRHtoLH()
{
	//* Not the fastest method, but it works...
	matMatrix4x4 mSwap = *this;
	_12 = mSwap._13;
	_13 = mSwap._12;

	_21 = mSwap._31;
	_22 = mSwap._33;
	_23 = mSwap._32;

	_31 = mSwap._21;
	_32 = mSwap._23;
	_33 = mSwap._22;

	_42 = mSwap._43;
	_43 = mSwap._42;
}


inline matMatrix4x4& matMatrix4x4::StoreAffineTransform( float scaling, const matVector3& RotationCenter, const matQuaternion& Rotation, const matVector3& Translation )
{
	matBaseMatrixAffineTransformation(	this, scaling, &RotationCenter, &Rotation, &Translation);
	return *this;
}


inline void matMatrix4x4::StoreOrthographicLH( float Width, float Height, float MinZ, float MaxZ )
{
	matBaseMatrixOrthoLH(this, Width, Height, MinZ, MaxZ);
}

inline void	matMatrix4x4::StoreOrthographicRH( float Width, float Height, float MinZ, float MaxZ )
{
	matBaseMatrixOrthoRH(this, Width, Height, MinZ, MaxZ);
}

inline void	matMatrix4x4::StoreOrthographicOffCenterLH(	float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ )
{
	matBaseMatrixOrthoOffCenterLH(this, MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
}

inline void matMatrix4x4::StoreOrthographicOffCenterRH(	float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ )
{
	matBaseMatrixOrthoOffCenterRH(this, MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveLH(float Width, float Height, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveLH(this, Width, Height, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveRH(float Width, float Height, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveRH(this, Width, Height, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveFovLH( float FovY, float Aspect, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveFovLH(this, FovY, Aspect, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveFovRH( float FovY, float Aspect, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveFovRH(this, FovY, Aspect, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveOffCenterLH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveOffCenterLH(this, MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
}

inline void matMatrix4x4::StorePerspectiveOffCenterRH( float MinX, float MaxX, float MinY, float MaxY, float MinZ, float MaxZ )
{
	matBaseMatrixPerspectiveOffCenterRH(this, MinX, MaxX, MinY, MaxY, MinZ, MaxZ);
}

inline matMatrix4x4& matMatrix4x4::StorePlaneProject( const matVector4& LightVec, const matPlane& Plane )
{
	matBaseMatrixShadow(this, &LightVec, &Plane);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreTransformation(
	const matVector3& ScaleCenter, const matQuaternion& ScalingRotation, const matVector3& Scaling,
	const matVector3& RotationCenter, const matQuaternion& Rotation,
	const matVector3& Translation )
{
	matBaseMatrixTransformation( this, &ScaleCenter, &ScalingRotation, &Scaling, &RotationCenter, &Rotation, &Translation);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreTransformation( const matVector3& Scaling,
                                                        const matQuaternion& Rotation,
                                                        const matVector3& Translation )
{
    matBaseMatrixTransformation( this, &Scaling, &Rotation, &Translation);
    return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreUVTransformation(
	const matVector2 &Scaling, 
	const matVector3 &Rotation,
	const matVector2 &Translation )
{
	// FIX: make this faster later
    StoreIdentity();
    _31 = Translation.x - 0.5f;
    _32 = Translation.y - 0.5f;

    float halfR = Rotation.z * 0.5f;
    matBaseQUATERNION q(0,0,std::sin(halfR),std::cos(halfR));
    matMatrix4x4 wAngle;
    matBaseMatrixRotationQuaternion(&wAngle, &q);
    Multiply(wAngle);

    matMatrix4x4 tiling(matIdentity);
    tiling._11 = Scaling.x;
    tiling._22 = Scaling.y;
    Multiply(tiling);

    _31 += 0.5f;
    _32 += 0.5f;

    return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreTranslation( const matVector3& Translation )
{
	matBaseMatrixTranslation(this, Translation.x, Translation.y, Translation.z);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreTranslation2D( const matVector2& Translation )
{
	StoreIdentity();
	_31 = Translation.x;
	_32 = Translation.y;
	return *this;
}

inline matMatrix4x4& matMatrix4x4::Multiply( const matMatrix4x4 &mat )
{
	matMatrix4x4 temp(*this);
	StoreMultiply(temp, mat);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::PreMultiply( const matMatrix4x4 &mat )
{
    matMatrix4x4 temp(*this);
    StoreMultiply(mat, temp);
    return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreMultiply( const matMatrix4x4 &mat1, const matMatrix4x4 &mat2 )
{
	matBaseMatrixMultiply(this, &mat1, &mat2);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::StoreMultiplyTranspose(IN const matMatrix4x4 &mat1, IN const matMatrix4x4 &mat2 )
{
	matBaseMatrixMultiplyTranspose(this, &mat1, &mat2);
	return *this;
}

inline float matMatrix4x4::CalcDeterminant() const
{
	return matBaseMatrixDeterminant(this);
}

inline matMatrix4x4& matMatrix4x4::Invert( OUT float *pfDet )
{
	matMatrix4x4 temp(*this);
	matBaseMatrixInverse(this, pfDet, &temp);
	return *this;
}

inline matMatrix4x4& matMatrix4x4::InvertFast()
{
	errAssertV( fabsf(1.0f - GetRow0().MagnitudeSquared()) < 0.1f, 
        ("Cannot call InvertFast() on matrix with scaling"));
	float fTemp=_12; _12=_21; _21=fTemp;
	fTemp=_13; _13=_31; _31=fTemp;
	fTemp=_23; _23=_32; _32=fTemp;

	matVector3 pos(-GetPosition());
	matBaseVec3TransformNormal(&GetPosition(), &pos, this);
	return *this;
}


inline matVector3 matMatrix4x4::TransformPoint( IN const matVector3& vPoint ) const
{
	matVector3 v3Out; 
	matBaseVec3TransformCoord(&v3Out, &vPoint, this);
	return v3Out;
}

inline matVector3 matMatrix4x4::TransformVector(const matVector3& vNormal ) const
{
	matVector3 v3Out; 
	matBaseVec3TransformNormal(&v3Out, &vNormal, this);
	return v3Out;
}

inline matVector4 matMatrix4x4::Transform( IN const matVector4& vIn ) const
{
	matVector4 vOut; 
	matBaseVec4Transform(&vOut, &vIn, this);
	return vOut;
}


// global functions
inline matMatrix4x4 operator*( const matMatrix4x4& lhs, const matMatrix4x4& rhs )
{
	matMatrix4x4 r;
	r.StoreMultiply(lhs, rhs);
	return r;
}

inline matMatrix4x4 Invert(const matMatrix4x4& mat)
{
	matMatrix4x4 temp(mat);
	temp.Invert();
	return temp;
}

#endif // INCLUDED_matMatrix4x4
