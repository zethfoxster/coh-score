/**
*   Created: 2004/10/04
*   Copyright: 2004, NCSoft. All Rights Reserved
*   Author: Quoc Tran
*
*   @par Last modified
*      $DateTime: 2007/06/25 16:48:35 $
*      $Author: pflesher $
*      $Change: 579968 $
*   @file
*
*/

#ifndef   INCLUDED_matMatrix4x3T
#define   INCLUDED_matMatrix4x3T
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if CORE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4201) // anonymous unions warning
#endif

#include "arda2/math/matTypes.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matVector4.h"
#include "arda2/math/matMatrix4x4.h"
#include "arda2/math/matPlane.h"
#include "arda2/math/matQuaternion.h"

#include <algorithm>

class matQuaternion;

/**
 *  This is meant to store the transposed information of a normal 4x4 matrix. Also omits
 *  the last row (since in a transposed matrix, the last row is comprised of zeros anyway.
 */

#if CORE_COMPILER_MSVC
__declspec(align(16))
#endif
class matMatrix4x3T
{
public:

    union
    {
        struct 
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
        };
        float m[3][4];
    };

    matMatrix4x3T() {};
    matMatrix4x3T( matIdentityType );
    matMatrix4x3T(const matMatrix4x3T& mat)
    { memcpy(&_11, &mat, sizeof(matMatrix4x3T)); }
    explicit matMatrix4x3T(const matMatrix4x4& mat) :
        _11(mat._11), _12(mat._21), _13(mat._31), _14(mat._41),
        _21(mat._12), _22(mat._22), _23(mat._32), _24(mat._42),
        _31(mat._13), _32(mat._23), _33(mat._33), _34(mat._43) {};

    matMatrix4x3T( float f11, float f12, float f13, float f14,
                   float f21, float f22, float f23, float f24,
                   float f31, float f32, float f33, float f34 ) :
        _11(f11), _12(f12), _13(f13), _14(f14),
        _21(f21), _22(f22), _23(f23), _24(f24),
        _31(f31), _32(f32), _33(f33), _34(f34) {};

    const matVector4&	GetRow0() const;
    const matVector4&	GetRow1() const;
    const matVector4&	GetRow2() const;
    const matVector4&	GetRow3() const;

    matVector4&			GetRow0();
    matVector4&			GetRow1();
    matVector4&			GetRow2();
    matVector4&			GetRow3();

    const matVector3	GetPosition() const;
    matVector3			GetPosition();
    void				SetPosition( IN const matVector3& v3Position );

    matMatrix4x3T&      Multiply( const matMatrix4x3T &mat );

    matMatrix4x3T&		StoreEulerAngles( float fYaw, float fPitch, float fRoll );
    matMatrix4x3T&		StoreQuaternion( const matQuaternion &rOrientation );
    matMatrix4x3T&		StoreReflect( const matPlane& ReflectPlane );
    matMatrix4x3T&      StoreMatrix(const matMatrix4x4& rhs);

    matMatrix4x3T&      StoreMultiply( const matMatrix4x3T &mat1, const matMatrix4x3T &mat2 );
};

inline matMatrix4x3T::matMatrix4x3T( matIdentityType ) :
    _11(1), _12(0), _13(0), _14(0),
    _21(0), _22(1), _23(0), _24(0),
    _31(0), _32(0), _33(1), _34(0) {};
inline matMatrix4x3T& matMatrix4x3T::StoreMatrix(const matMatrix4x4& rhs)
{ 
    // transposed copy
    _11 = rhs._11; _12 = rhs._21; _13 = rhs._31; _14 = rhs._41;
    _21 = rhs._12; _22 = rhs._22; _23 = rhs._32; _24 = rhs._42;
    _31 = rhs._13; _32 = rhs._23; _33 = rhs._33; _34 = rhs._43;
    return *this;
}

inline const matVector4& matMatrix4x3T::GetRow0() const
{
    return *((matVector4*)(&_11));
}

inline const matVector4& matMatrix4x3T::GetRow1() const
{
    return *((matVector4*)(&_21));
}

inline const matVector4& matMatrix4x3T::GetRow2() const
{
    return *((matVector4*)(&_31));
}

inline matVector4& matMatrix4x3T::GetRow0()
{
    return *((matVector4*)(&_11));
}

inline matVector4& matMatrix4x3T::GetRow1() 
{
    return *((matVector4*)(&_21));
}

inline matVector4& matMatrix4x3T::GetRow2()
{
    return *((matVector4*)(&_31));
}

inline const matVector3 matMatrix4x3T::GetPosition() const
{
    const matVector3 pos( _14, _24, _34 );
    return pos;
}

inline matVector3 matMatrix4x3T::GetPosition()
{
    matVector3 pos( _14, _24, _34 );
    return pos;
}

inline void matMatrix4x3T::SetPosition( IN const matVector3& v3Position )
{
    _14 = v3Position.x;
    _24 = v3Position.y;
    _34 = v3Position.z;
}

inline matMatrix4x3T& matMatrix4x3T::Multiply( const matMatrix4x3T &mat )
{
    errAssertV(this!=&mat, ("Cannot call matMatrix4x3T::Multiply() with 'this' as the parameter"));
//    matMatrix4x3T temp(*this); // This fails to compile in x64 because of compiler warning about alignment...
    matMatrix4x3T temp;
    memcpy(&temp._11, this, sizeof(matMatrix4x3T));

    // mat*temp since it's transposed
    _11 = (mat._11 * temp._11) + (mat._12 * temp._21) + (mat._13 * temp._31);
    _12 = (mat._11 * temp._12) + (mat._12 * temp._22) + (mat._13 * temp._32);
    _13 = (mat._11 * temp._13) + (mat._12 * temp._23) + (mat._13 * temp._33);
    _14 = (mat._11 * temp._14) + (mat._12 * temp._24) + (mat._13 * temp._34) + mat._14;

    _21 = (mat._21 * temp._11) + (mat._22 * temp._21) + (mat._23 * temp._31);
    _22 = (mat._21 * temp._12) + (mat._22 * temp._22) + (mat._23 * temp._32);
    _23 = (mat._21 * temp._13) + (mat._22 * temp._23) + (mat._23 * temp._33);
    _24 = (mat._21 * temp._14) + (mat._22 * temp._24) + (mat._23 * temp._34) + mat._24;

    _31 = (mat._31 * temp._11) + (mat._32 * temp._21) + (mat._33 * temp._31);
    _32 = (mat._31 * temp._12) + (mat._32 * temp._22) + (mat._33 * temp._32);
    _33 = (mat._31 * temp._13) + (mat._32 * temp._23) + (mat._33 * temp._33);
    _34 = (mat._31 * temp._14) + (mat._32 * temp._24) + (mat._33 * temp._34) + mat._34;

    return *this;
}

inline matMatrix4x3T& matMatrix4x3T::StoreMultiply( const matMatrix4x3T &mat1, const matMatrix4x3T &mat2 )
{
    errAssertV(this!=&mat1, ("Cannot call matMatrix4x3T::Multiply() with 'this' as the parameter"));
    errAssertV(this!=&mat2, ("Cannot call matMatrix4x3T::Multiply() with 'this' as the parameter"));

    // mat*temp since it's transposed
    _11 = (mat2._11 * mat1._11) + (mat2._12 * mat1._21) + (mat2._13 * mat1._31);
    _12 = (mat2._11 * mat1._12) + (mat2._12 * mat1._22) + (mat2._13 * mat1._32);
    _13 = (mat2._11 * mat1._13) + (mat2._12 * mat1._23) + (mat2._13 * mat1._33);
    _14 = (mat2._11 * mat1._14) + (mat2._12 * mat1._24) + (mat2._13 * mat1._34) + mat2._14;

    _21 = (mat2._21 * mat1._11) + (mat2._22 * mat1._21) + (mat2._23 * mat1._31);
    _22 = (mat2._21 * mat1._12) + (mat2._22 * mat1._22) + (mat2._23 * mat1._32);
    _23 = (mat2._21 * mat1._13) + (mat2._22 * mat1._23) + (mat2._23 * mat1._33);
    _24 = (mat2._21 * mat1._14) + (mat2._22 * mat1._24) + (mat2._23 * mat1._34) + mat2._24;

    _31 = (mat2._31 * mat1._11) + (mat2._32 * mat1._21) + (mat2._33 * mat1._31);
    _32 = (mat2._31 * mat1._12) + (mat2._32 * mat1._22) + (mat2._33 * mat1._32);
    _33 = (mat2._31 * mat1._13) + (mat2._32 * mat1._23) + (mat2._33 * mat1._33);
    _34 = (mat2._31 * mat1._14) + (mat2._32 * mat1._24) + (mat2._33 * mat1._34) + mat2._34;

    return *this;
}

#if CORE_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif // INCLUDED_matMatrix4x3T
