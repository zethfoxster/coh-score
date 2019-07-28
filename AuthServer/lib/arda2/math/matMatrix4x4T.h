/**
*   Created: 2004/10/04
*   Copyright: 2004, NCSoft. All Rights Reserved
*   Author: Quoc Tran
*
*   @par Last modified
*      $DateTime: 2006/07/12 14:21:10 $
*      $Author: cthurow $
*      $Change: 273860 $
*   @file
*
*/

#ifndef   INCLUDED_matMatrix4x4T
#define   INCLUDED_matMatrix4x4T
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

/**
 *  4x4 matrix that stores the transposed information of a normal 4x4 matrix
 */

#if CORE_COMPILER_MSVC
__declspec(align(16))
#endif
class matMatrix4x4T : public matBaseMATRIX
{
public:
    matMatrix4x4T() : matBaseMATRIX() {};
    matMatrix4x4T( matIdentityType );
    explicit matMatrix4x4T(const matBaseMATRIX &m) : matBaseMATRIX(m) {};
    matMatrix4x4T(const matMatrix4x4T &m) : matBaseMATRIX(m) {};
    explicit matMatrix4x4T(const matMatrix4x4& mat);
    matMatrix4x4T( float f11, float f12, float f13, float f14,
                   float f21, float f22, float f23, float f24,
                   float f31, float f32, float f33, float f34,
                   float f41, float f42, float f43, float f44);


    // FIXME[pmf]: Hmm... these methods are misleadingly named, as they DON'T return the row, just part of the orientation submatrix
    const matVector3&	GetRow0() const;
    const matVector3&	GetRow1() const;
    const matVector3&	GetRow2() const;
    const matVector3&	GetRow3() const;

    matVector3&			GetRow0();
    matVector3&			GetRow1();
    matVector3&			GetRow2();
    matVector3&			GetRow3();

    const matVector3	GetPosition() const;
    matVector3			GetPosition();
    void				SetPosition( IN const matVector3& v3Position );

    matMatrix4x4T&      Multiply( const matMatrix4x4T &mat );

    matMatrix4x4T&		StoreIdentity();
    matMatrix4x4T&      StoreMatrix(const matMatrix4x4& rhs);
    matMatrix4x4T&      StoreMultiply( const matMatrix4x4T &mat1, const matMatrix4x4T &mat2 );
    matMatrix4x4T&      StoreMultiply( const matMatrix4x3T &mat1, const matMatrix4x4T &mat2 );
    matMatrix4x4T&		StoreQuaternion( const matQuaternion &rOrientation );

    // texture coordinate transform matrices
    matMatrix4x4T&		StoreUVTransformation( const matVector2 &Scaling, 
                                               const float      &fRotation,
                                               const matVector2 &Translation );

    // projection matrices
    void	StoreOrthographicLH( float Width, float Height, float MinZ, float MaxZ );
    void	StoreOrthographicRH( float Width, float Height, float MinZ, float MaxZ );

    void	StorePerspectiveLH(float Width, float Height, float MinZ, float MaxZ );
    void	StorePerspectiveRH(float Width, float Height, float MinZ, float MaxZ );
};

// inlines
inline matMatrix4x4T::matMatrix4x4T( matIdentityType ) :
    matBaseMATRIX(1,0,0,0,
               0,1,0,0,
               0,0,1,0,
               0,0,0,1) {};

inline matMatrix4x4T::matMatrix4x4T( float f11, float f12, float f13, float f14,
                                     float f21, float f22, float f23, float f24,
                                     float f31, float f32, float f33, float f34,
                                     float f41, float f42, float f43, float f44 ) : 
    matBaseMATRIX(f11,f12,f13,f14,
               f21,f22,f23,f24,
               f31,f32,f33,f34,
               f41,f42,f43,f44) {};
    
inline matMatrix4x4T::matMatrix4x4T(const matMatrix4x4& mat) : 
    matBaseMATRIX(mat._11,mat._21,mat._31,mat._41,
               mat._12,mat._22,mat._32,mat._42,
               mat._13,mat._23,mat._33,mat._43,
               mat._14,mat._24,mat._34,mat._44) {};

inline matMatrix4x4T& matMatrix4x4T::StoreMatrix(const matMatrix4x4& rhs)
{ 
    // transposed copy
    _11 = rhs._11; _12 = rhs._21; _13 = rhs._31; _14 = rhs._41;
    _21 = rhs._12; _22 = rhs._22; _23 = rhs._32; _24 = rhs._42;
    _31 = rhs._13; _32 = rhs._23; _33 = rhs._33; _34 = rhs._43;
    _41 = rhs._14; _42 = rhs._24; _43 = rhs._34; _44 = rhs._44;
    return *this;
}


inline const matVector3& matMatrix4x4T::GetRow0() const
{
    return *((matVector3*)(&_11));
}

inline const matVector3& matMatrix4x4T::GetRow1() const
{
    return *((matVector3*)(&_21));
}

inline const matVector3& matMatrix4x4T::GetRow2() const
{
    return *((matVector3*)(&_31));
}

inline const matVector3& matMatrix4x4T::GetRow3() const
{
    return *((matVector3*)(&_41));
}

inline matVector3& matMatrix4x4T::GetRow0()
{
    return *((matVector3*)(&_11));
}

inline matVector3& matMatrix4x4T::GetRow1() 
{
    return *((matVector3*)(&_21));
}

inline matVector3& matMatrix4x4T::GetRow2()
{
    return *((matVector3*)(&_31));
}

inline matVector3& matMatrix4x4T::GetRow3()
{
    return *((matVector3*)(&_41));
}

inline const matVector3 matMatrix4x4T::GetPosition() const
{
    const matVector3 pos( _14, _24, _34 );
    return pos;
}

inline matVector3 matMatrix4x4T::GetPosition()
{
    matVector3 pos( _14, _24, _34 );
    return pos;
}

inline void matMatrix4x4T::SetPosition( IN const matVector3& v3Position )
{
    _14 = v3Position.x;
    _24 = v3Position.y;
    _34 = v3Position.z;
}


inline matMatrix4x4T& matMatrix4x4T::Multiply( const matMatrix4x4T &mat )
{
    errAssertV(this!=&mat, ("Cannot call matMatrix4x4T::Multiply() with 'this' as the parameter"));
    matMatrix4x4T temp(*this);

    // mat*temp since it's transposed
    _11 = (mat._11 * temp._11) + (mat._12 * temp._21) + (mat._13 * temp._31) + (mat._14 * temp._41);
    _12 = (mat._11 * temp._12) + (mat._12 * temp._22) + (mat._13 * temp._32) + (mat._14 * temp._42);
    _13 = (mat._11 * temp._13) + (mat._12 * temp._23) + (mat._13 * temp._33) + (mat._14 * temp._43);
    _14 = (mat._11 * temp._14) + (mat._12 * temp._24) + (mat._13 * temp._34) + (mat._14 * temp._44);

    _21 = (mat._21 * temp._11) + (mat._22 * temp._21) + (mat._23 * temp._31) + (mat._24 * temp._41);
    _22 = (mat._21 * temp._12) + (mat._22 * temp._22) + (mat._23 * temp._32) + (mat._24 * temp._42);
    _23 = (mat._21 * temp._13) + (mat._22 * temp._23) + (mat._23 * temp._33) + (mat._24 * temp._43);
    _24 = (mat._21 * temp._14) + (mat._22 * temp._24) + (mat._23 * temp._34) + (mat._24 * temp._44);

    _31 = (mat._31 * temp._11) + (mat._32 * temp._21) + (mat._33 * temp._31) + (mat._34 * temp._41);
    _32 = (mat._31 * temp._12) + (mat._32 * temp._22) + (mat._33 * temp._32) + (mat._34 * temp._42);
    _33 = (mat._31 * temp._13) + (mat._32 * temp._23) + (mat._33 * temp._33) + (mat._34 * temp._43);
    _34 = (mat._31 * temp._14) + (mat._32 * temp._24) + (mat._33 * temp._34) + (mat._34 * temp._44);

    _41 = (mat._41 * temp._11) + (mat._42 * temp._21) + (mat._43 * temp._31) + (mat._44 * temp._41);
    _42 = (mat._41 * temp._12) + (mat._42 * temp._22) + (mat._43 * temp._32) + (mat._44 * temp._42);
    _43 = (mat._41 * temp._13) + (mat._42 * temp._23) + (mat._43 * temp._33) + (mat._44 * temp._43);
    _44 = (mat._41 * temp._14) + (mat._42 * temp._24) + (mat._43 * temp._34) + (mat._44 * temp._44);

    return *this;
}


inline matMatrix4x4T& matMatrix4x4T::StoreIdentity()
{
    *this = *((matMatrix4x4T*)&s_identityMat);
    return *this;
}


inline matMatrix4x4T& matMatrix4x4T::StoreMultiply( const matMatrix4x4T &mat1, const matMatrix4x4T &mat2 )
{
    errAssertV(this!=&mat1, ("Cannot call matMatrix4x4T::StoreMultiply() with 'this' as one of the parameters - Use ::Multiply() instead"));
    errAssertV(this!=&mat2, ("Cannot call matMatrix4x4T::StoreMultiply() with 'this' as one of the parameters - Use ::Multiply() instead"));
  
    // mat2*mat1 since it's transposed
    _11 = (mat2._11 * mat1._11) + (mat2._12 * mat1._21) + (mat2._13 * mat1._31) + (mat2._14 * mat1._41);
    _12 = (mat2._11 * mat1._12) + (mat2._12 * mat1._22) + (mat2._13 * mat1._32) + (mat2._14 * mat1._42);
    _13 = (mat2._11 * mat1._13) + (mat2._12 * mat1._23) + (mat2._13 * mat1._33) + (mat2._14 * mat1._43);
    _14 = (mat2._11 * mat1._14) + (mat2._12 * mat1._24) + (mat2._13 * mat1._34) + (mat2._14 * mat1._44);

    _21 = (mat2._21 * mat1._11) + (mat2._22 * mat1._21) + (mat2._23 * mat1._31) + (mat2._24 * mat1._41);
    _22 = (mat2._21 * mat1._12) + (mat2._22 * mat1._22) + (mat2._23 * mat1._32) + (mat2._24 * mat1._42);
    _23 = (mat2._21 * mat1._13) + (mat2._22 * mat1._23) + (mat2._23 * mat1._33) + (mat2._24 * mat1._43);
    _24 = (mat2._21 * mat1._14) + (mat2._22 * mat1._24) + (mat2._23 * mat1._34) + (mat2._24 * mat1._44);

    _31 = (mat2._31 * mat1._11) + (mat2._32 * mat1._21) + (mat2._33 * mat1._31) + (mat2._34 * mat1._41);
    _32 = (mat2._31 * mat1._12) + (mat2._32 * mat1._22) + (mat2._33 * mat1._32) + (mat2._34 * mat1._42);
    _33 = (mat2._31 * mat1._13) + (mat2._32 * mat1._23) + (mat2._33 * mat1._33) + (mat2._34 * mat1._43);
    _34 = (mat2._31 * mat1._14) + (mat2._32 * mat1._24) + (mat2._33 * mat1._34) + (mat2._34 * mat1._44);

    _41 = (mat2._41 * mat1._11) + (mat2._42 * mat1._21) + (mat2._43 * mat1._31) + (mat2._44 * mat1._41);
    _42 = (mat2._41 * mat1._12) + (mat2._42 * mat1._22) + (mat2._43 * mat1._32) + (mat2._44 * mat1._42);
    _43 = (mat2._41 * mat1._13) + (mat2._42 * mat1._23) + (mat2._43 * mat1._33) + (mat2._44 * mat1._43);
    _44 = (mat2._41 * mat1._14) + (mat2._42 * mat1._24) + (mat2._43 * mat1._34) + (mat2._44 * mat1._44);

    return *this;
}


inline matMatrix4x4T& matMatrix4x4T::StoreMultiply( const matMatrix4x3T &mat1, const matMatrix4x4T &mat2 )
{
    errAssertV(this!=&mat2, ("Cannot call matMatrix4x4T::StoreMultiply() with 'this' as one of the parameters"));

    // mat2*mat1 since it's transposed
    _11 = (mat2._11 * mat1._11) + (mat2._12 * mat1._21) + (mat2._13 * mat1._31);
    _12 = (mat2._11 * mat1._12) + (mat2._12 * mat1._22) + (mat2._13 * mat1._32);
    _13 = (mat2._11 * mat1._13) + (mat2._12 * mat1._23) + (mat2._13 * mat1._33);
    _14 = (mat2._11 * mat1._14) + (mat2._12 * mat1._24) + (mat2._13 * mat1._34) + (mat2._14);

    _21 = (mat2._21 * mat1._11) + (mat2._22 * mat1._21) + (mat2._23 * mat1._31);
    _22 = (mat2._21 * mat1._12) + (mat2._22 * mat1._22) + (mat2._23 * mat1._32);
    _23 = (mat2._21 * mat1._13) + (mat2._22 * mat1._23) + (mat2._23 * mat1._33);
    _24 = (mat2._21 * mat1._14) + (mat2._22 * mat1._24) + (mat2._23 * mat1._34) + (mat2._24);

    _31 = (mat2._31 * mat1._11) + (mat2._32 * mat1._21) + (mat2._33 * mat1._31);
    _32 = (mat2._31 * mat1._12) + (mat2._32 * mat1._22) + (mat2._33 * mat1._32);
    _33 = (mat2._31 * mat1._13) + (mat2._32 * mat1._23) + (mat2._33 * mat1._33);
    _34 = (mat2._31 * mat1._14) + (mat2._32 * mat1._24) + (mat2._33 * mat1._34) + (mat2._34);

    _41 = (mat2._41 * mat1._11) + (mat2._42 * mat1._21) + (mat2._43 * mat1._31);
    _42 = (mat2._41 * mat1._12) + (mat2._42 * mat1._22) + (mat2._43 * mat1._32);
    _43 = (mat2._41 * mat1._13) + (mat2._42 * mat1._23) + (mat2._43 * mat1._33);
    _44 = (mat2._41 * mat1._14) + (mat2._42 * mat1._24) + (mat2._43 * mat1._34) + (mat2._44);

    return *this;
}

inline matMatrix4x4T& matMatrix4x4T::StoreQuaternion( const matQuaternion &rOrientation ) 
{
    // normalize input (necessary?)
    matBaseQUATERNION q;
    matBaseQuaternionNormalize(&q, (matBaseQUATERNION*)&rOrientation);

    // calculate coefficients
    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;
    float xx = q.x * x2;
    float xy = q.x * y2;
    float xz = q.x * z2;
    float yy = q.y * y2;
    float yz = q.y * z2;
    float zz = q.z * z2;
    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    // fill in matrix
    _11 = 1.0f - (yy + zz);
    _12 = xy - wz;
    _13 = xz + wy;
    _14 = 0.0f;

    _21 = xy + wz;
    _22 = 1.0f - (xx + zz);
    _23 = yz - wx;
    _24 = 0.0f;

    _31 = xz - wy;
    _32 = yz + wx;
    _33 = 1.0f - (xx + yy);
    _34 = 0.0f;

    _41 = 0.0f;
    _42 = 0.0f;
    _43 = 0.0f;
    _44 = 1.0f;

    return *this;
}


inline matMatrix4x4T& matMatrix4x4T::StoreUVTransformation(
    const matVector2 &Scaling, 
    const float      &fRotation,
    const matVector2 &Translation )
{
    // FIX: make this faster later
    StoreIdentity();
    _13 = Translation.x - 0.5f;
    _23 = Translation.y - 0.5f;

    float halfR = fRotation * 0.5f;
    matBaseQUATERNION q(0,0,std::sin(halfR),std::cos(halfR));
    matMatrix4x4T wAngle;
    wAngle.StoreQuaternion(q);
    Multiply(wAngle);

    matMatrix4x4T tiling(matIdentity);
    tiling._11 = Scaling.x;
    tiling._22 = Scaling.y;
    Multiply(tiling);

    _13 += 0.5f;
    _23 += 0.5f;

    return *this;
}


/**
 *  Calculate the left-hand orthographic projection matrix, and then switch values in the matrix
 *  to account for the transposed version of the matrix.
 */
inline void matMatrix4x4T::StoreOrthographicLH( float Width, float Height, float MinZ, float MaxZ )
{
    matBaseMatrixOrthoLH(this, Width, Height, MinZ, MaxZ);
    _34 = _43;
    _43 = 0;
}

/**
*  Calculate the right-hand orthographic projection matrix, and then switch values in the matrix
*  to account for the transposed version of the matrix.
*/
inline void	matMatrix4x4T::StoreOrthographicRH( float Width, float Height, float MinZ, float MaxZ )
{
    matBaseMatrixOrthoRH(this, Width, Height, MinZ, MaxZ);
    _34 = _43;
    _43 = 0;    
}

/**
*  Calculate the left-hand perspective projection matrix, and then switch values in the matrix
*  to account for the transposed version of the matrix.
*/
inline void matMatrix4x4T::StorePerspectiveLH(float Width, float Height, float MinZ, float MaxZ )
{
    matBaseMatrixPerspectiveLH(this, Width, Height, MinZ, MaxZ);
    _34 = _43;
    _43 = 1;
}

/**
*  Calculate the right-hand perspective projection matrix, and then switch values in the matrix
*  to account for the transposed version of the matrix.
*/
inline void matMatrix4x4T::StorePerspectiveRH(float Width, float Height, float MinZ, float MaxZ )
{
    matBaseMatrixPerspectiveRH(this, Width, Height, MinZ, MaxZ);
    _34 = _43;
    _43 = -1;
}

#endif // INCLUDED_matMatrix4x4T
